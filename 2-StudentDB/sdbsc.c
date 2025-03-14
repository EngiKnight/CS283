#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> //c library for system call file routines
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

// database include files
#include "db.h"
#include "sdbsc.h"

/*
 *  open_db
 *      dbFile:  name of the database file
 *      should_truncate:  indicates if opening the file also empties it
 *
 *  returns:  File descriptor on success, or ERR_DB_FILE on failure
 *
 *  console:  Does not produce any console I/O on success
 *            M_ERR_DB_OPEN on error
 *
 */
int open_db(char *dbFile, bool should_truncate)
{
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    int flags = O_RDWR | O_CREAT;
    if (should_truncate)
        flags |= O_TRUNC;

    int fd = open(dbFile, flags, mode);
    if (fd == -1)
    {
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }

    return fd;
}

/*
 *  get_student
 *      fd:  linux file descriptor
 *      id:  the student id we are looking forname of the
 *      *s:  a pointer where the located (if found) student data will be
 *           copied
 *
 *  returns:  NO_ERROR       student located and copied into *s
 *            ERR_DB_FILE    database file I/O issue
 *            SRCH_NOT_FOUND student was not located in the database
 *
 *  console:  Does not produce any console I/O used by other functions
 */
int get_student(int fd, int id, student_t *s)
{
    off_t offset = (off_t)id * STUDENT_RECORD_SIZE;
    off_t pos = lseek(fd, offset, SEEK_SET);
    if (pos == (off_t)-1)
    {
        return ERR_DB_FILE;
    }

    student_t temp = {0};
    ssize_t bytes_read = read(fd, &temp, STUDENT_RECORD_SIZE);
    if (bytes_read < 0)
    {
        return ERR_DB_FILE;
    }
    if (bytes_read == 0)
    {
        return SRCH_NOT_FOUND;
    }
    if (bytes_read < STUDENT_RECORD_SIZE)
    {
        return ERR_DB_FILE;
    }

    if (memcmp(&temp, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) == 0)
    {
        return SRCH_NOT_FOUND;
    }

    *s = temp;
    return NO_ERROR;
}

/*
 *  add_student
 *      fd:     linux file descriptor
 *      id:     student id (range is defined in db.h )
 *      fname:  student first name
 *      lname:  student last name
 *      gpa:    GPA as an integer (range defined in db.h)
 *
 *  returns:  NO_ERROR       student added to database
 *            ERR_DB_FILE    database file I/O issue
 *            ERR_DB_OP      database operation logically failed (aka student
 *                           already exists)
 *
 *
 *  console:  M_STD_ADDED       on success
 *            M_ERR_DB_ADD_DUP  student already exists
 *            M_ERR_DB_READ     error reading or seeking the database file
 *            M_ERR_DB_WRITE    error writing to db file (adding student)
 *
 */
int add_student(int fd, int id, char *fname, char *lname, int gpa)
{
    off_t offset = (off_t)id * STUDENT_RECORD_SIZE;
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1)
    {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }

    student_t temp = {0};
    ssize_t bytes_read = read(fd, &temp, STUDENT_RECORD_SIZE);
    if (bytes_read < 0)
    {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }
    else if (bytes_read > 0)
    {
        if (bytes_read < STUDENT_RECORD_SIZE)
        {
            printf(M_ERR_DB_READ);
            return ERR_DB_FILE;
        }
        if (memcmp(&temp, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) != 0)
        {
            printf(M_ERR_DB_ADD_DUP, id);
            return ERR_DB_OP;
        }
    }

    student_t new_student = {0};
    new_student.id = id;
    strncpy(new_student.fname, fname, sizeof(new_student.fname) - 1);
    strncpy(new_student.lname, lname, sizeof(new_student.lname) - 1);
    new_student.gpa = gpa;

    if (lseek(fd, offset, SEEK_SET) == (off_t)-1)
    {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }

    ssize_t written = write(fd, &new_student, STUDENT_RECORD_SIZE);
    if (written < 0 || written < STUDENT_RECORD_SIZE)
    {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }

    printf(M_STD_ADDED, id);
    return NO_ERROR;
}

/*
 *  del_student
 *      fd:     linux file descriptor
 *      id:     student id to be deleted
 *
 *  returns:  NO_ERROR       student deleted from database
 *            ERR_DB_FILE    database file I/O issue
 *            ERR_DB_OP      database operation logically failed (aka student
 *                           not in database)
 *
 *
 *  console:  M_STD_DEL_MSG      on success
 *            M_STD_NOT_FND_MSG  student not in database, cant be deleted
 *            M_ERR_DB_READ      error reading or seeking the database file
 *            M_ERR_DB_WRITE     error writing to db file (adding student)
 *
 */
int del_student(int fd, int id)
{
    student_t found = {0};
    int rc = get_student(fd, id, &found);
    if (rc == SRCH_NOT_FOUND)
    {
        printf(M_STD_NOT_FND_MSG, id);
        return ERR_DB_OP;
    }
    else if (rc == ERR_DB_FILE)
    {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }

    off_t offset = (off_t)id * STUDENT_RECORD_SIZE;
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1)
    {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }

    ssize_t written = write(fd, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE);
    if (written < 0 || written < STUDENT_RECORD_SIZE)
    {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }

    printf(M_STD_DEL_MSG, id);
    return NO_ERROR;
}

/*
 *  count_db_records
 *      fd:     linux file descriptor
 *
 *  returns:  <number>       returns the number of records in db on success
 *            ERR_DB_FILE    database file I/O issue
 *            ERR_DB_OP      database operation logically failed (aka student
 *                           not in database)
 *
 *
 *  console:  M_DB_RECORD_CNT  on success, to report the number of students in db
 *            M_DB_EMPTY       on success if the record count in db is zero
 *            M_ERR_DB_READ    error reading or seeking the database file
 *            M_ERR_DB_WRITE   error writing to db file (adding student)
 *
 */
int count_db_records(int fd)
{
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1)
    {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }

    int record_count = 0;
    student_t temp;
    while (1)
    {
        ssize_t bytes_read = read(fd, &temp, STUDENT_RECORD_SIZE);
        if (bytes_read < 0)
        {
            printf(M_ERR_DB_READ);
            return ERR_DB_FILE;
        }
        if (bytes_read == 0)
        {
            break;
        }
        if (bytes_read < STUDENT_RECORD_SIZE)
        {
            printf(M_ERR_DB_READ);
            return ERR_DB_FILE;
        }

        if (memcmp(&temp, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) != 0)
        {
            record_count++;
        }
    }

    if (record_count == 0)
    {
        printf(M_DB_EMPTY);
    }
    else
    {
        printf(M_DB_RECORD_CNT, record_count);
    }

    return record_count;
}

/*
 *  print_db
 *      fd:     linux file descriptor
 *
 *  Prints all records in the database.  Start by reading the
 *  database at the beginning, and continue reading individual records
 *  until you it EOF.  EOF is when the read() syscall returns 0. Check
 *  if a slot is empty or previously deleted by investigating if all of
 *  the bytes in the record read are zeros - I would suggest using memory
 *  compare memcmp() for this. Be careful as the database might be empty.
 *  on the first real row encountered print the header for the required output:
 *
 *     printf(STUDENT_PRINT_HDR_STRING, "ID",
 *                  "FIRST_NAME", "LAST_NAME", "GPA");
 *
 *  then for each valid record encountered print the required output:
 *
 *     printf(STUDENT_PRINT_FMT_STRING, student.id, student.fname,
 *                    student.lname, calculated_gpa_from_student);
 *
 *  returns:  NO_ERROR       on success
 *            ERR_DB_FILE    database file I/O issue
 *
 *
 *  console:  <see above>      on success, print table or database empty
 *            M_ERR_DB_READ    error reading or seeking the database file
 *
 */
int print_db(int fd)
{
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1)
    {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }

    int found_any = 0;
    student_t temp;
    while (1)
    {
        ssize_t bytes_read = read(fd, &temp, STUDENT_RECORD_SIZE);
        if (bytes_read < 0)
        {
            printf(M_ERR_DB_READ);
            return ERR_DB_FILE;
        }
        if (bytes_read == 0)
        {
            break;
        }
        if (bytes_read < STUDENT_RECORD_SIZE)
        {
            printf(M_ERR_DB_READ);
            return ERR_DB_FILE;
        }

        if (memcmp(&temp, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) != 0)
        {
            if (found_any == 0)
            {
                printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST_NAME", "LAST_NAME", "GPA");
            }
            found_any++;
            float real_gpa = temp.gpa / 100.0f;
            printf(STUDENT_PRINT_FMT_STRING, temp.id, temp.fname, temp.lname, real_gpa);
        }
    }

    if (found_any == 0)
    {
        printf(M_DB_EMPTY);
    }

    return NO_ERROR;
}

/*
 *  print_student
 *      *s:   a pointer to a student_t structure that should
 *            contain a valid student to be printed
 *
 *  Start by ensuring that provided student pointer is valid.  To do this
 *  make sure it is not NULL and that s->id is not zero.  After ensuring
 *  that the student is valid, print it the exact way that is described
 *  in the print_db() function by first printing the header then the
 *  student data:
 *
 *     printf(STUDENT_PRINT_HDR_STRING, "ID",
 *                  "FIRST NAME", "LAST_NAME", "GPA");
 *
 *     printf(STUDENT_PRINT_FMT_STRING, s->id, s->fname,
 *                    student.lname, calculated_gpa_from_s);
 *
 *  returns:  nothing, this is a void function
 *
 *
 *  console:  <see above>      on success, print table or database empty
 *            M_ERR_STD_PRINT  if the function argument s is NULL or if
 *                             s->id is zero
 *
 */
void print_student(student_t *s)
{
    if ((s == NULL) || (s->id == 0))
    {
        printf(M_ERR_STD_PRINT);
        return;
    }

    printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST NAME", "LAST_NAME", "GPA");
    float real_gpa = s->gpa / 100.0f;
    printf(STUDENT_PRINT_FMT_STRING, s->id, s->fname, s->lname, real_gpa);
}

/*
 *  NOTE IMPLEMENTING THIS FUNCTION IS EXTRA CREDIT
 *
 *  compress_db
 *      fd:     linux file descriptor
 *
 *  This assignment takes advantage of the way Linux handles sparse files
 *  on disk. Thus if there is a large hole between student records, Linux
 *  will not use any physical storage.  However, when a database record is
 *  deleted storage is used to write a blank - see EMPTY_STUDENT_RECORD from
 *  db.h - record.
 *
 *  Since Linux provides no way to delete data in the middle of a file, and
 *  deleted records take up physical storage, this function will compress the
 *  database by rewriting a new database file that only includes valid student
 *  records. There are a number of ways to do this, but since this is extra credit
 *  you need to figure this out on your own.
 *
 *  At a high level create a temporary database file then copy all valid students from
 *  the active database (passed in via fd) to the temporary file. When this is done
 *  rename the temporary database file to the name of the real database file. See
 *  the constants in db.h for required file names:
 *
 *         #define DB_FILE     "student.db"        //name of database file
 *         #define TMP_DB_FILE ".tmp_student.db"   //for extra credit
 *
 *  Note that you are passed in the fd of the database file to be compressed,
 *  it is very likely you will need to close it to overwrite it with the
 *  compressed version of the file.  To ensure the caller can work with the
 *  compressed file after you create it, it is a good design to return the fd
 *  of the new compressed file from this function
 *
 *  returns:  <number>       returns the fd of the compressed database file
 *            ERR_DB_FILE    database file I/O issue
 *
 *
 *  console:  M_DB_COMPRESSED_OK  on success, the db was successfully compressed.
 *            M_ERR_DB_OPEN    error when opening/creating temporary database file.
 *                             this error should also be returned after you
 *                             compressed the database file and if you are unable
 *                             to open it to pass the fd back to the caller
 *            M_ERR_DB_CREATE  error creating the db file. For instance the
 *                             inability to copy the temporary file back as
 *                             the primary database file.
 *            M_ERR_DB_READ    error reading or seeking the the db or tempdb file
 *            M_ERR_DB_WRITE   error writing to db file (adding student)
 *
 */
int compress_db(int fd)
{
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1)
    {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }

    int new_fd = open_db(TMP_DB_FILE, true);
    if (new_fd < 0)
    {
        return ERR_DB_FILE;
    }

    student_t temp;
    while (1)
    {
        ssize_t bytes_read = read(fd, &temp, STUDENT_RECORD_SIZE);
        if (bytes_read < 0)
        {
            printf(M_ERR_DB_READ);
            close(new_fd);
            close(fd);
            return ERR_DB_FILE;
        }
        if (bytes_read == 0)
        {
            break;
        }
        if (bytes_read < STUDENT_RECORD_SIZE)
        {
            printf(M_ERR_DB_READ);
            close(new_fd);
            close(fd);
            return ERR_DB_FILE;
        }

        if (memcmp(&temp, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) != 0)
        {
            off_t offset = (off_t)temp.id * STUDENT_RECORD_SIZE;
            if (lseek(new_fd, offset, SEEK_SET) == (off_t)-1)
            {
                printf(M_ERR_DB_WRITE);
                close(new_fd);
                close(fd);
                return ERR_DB_FILE;
            }
            ssize_t written = write(new_fd, &temp, STUDENT_RECORD_SIZE);
            if (written < 0 || written < STUDENT_RECORD_SIZE)
            {
                printf(M_ERR_DB_WRITE);
                close(new_fd);
                close(fd);
                return ERR_DB_FILE;
            }
        }
    }

    close(fd);
    close(new_fd);

    if (rename(TMP_DB_FILE, DB_FILE) != 0)
    {
        printf(M_ERR_DB_CREATE);
        return ERR_DB_FILE;
    }

    int compressed_fd = open_db(DB_FILE, false);
    if (compressed_fd < 0)
    {
        return ERR_DB_FILE;
    }

    printf(M_DB_COMPRESSED_OK);
    return compressed_fd;
}

/*
 *  validate_range
 *      id:  proposed student id
 *      gpa: proposed gpa
 *
 *  This function validates that the id and gpa are in the allowable ranges
 *  as per the specifications.  It checks if the values are within the
 *  inclusive range using constents in db.h
 *
 *  returns:    NO_ERROR       on success, both ID and GPA are in range
 *              EXIT_FAIL_ARGS if either ID or GPA is out of range
 *
 *  console:  This function does not produce any output
 *
 */
int validate_range(int id, int gpa)
{
    if ((id < MIN_STD_ID) || (id > MAX_STD_ID))
        return EXIT_FAIL_ARGS;

    if ((gpa < MIN_STD_GPA) || (gpa > MAX_STD_GPA))
        return EXIT_FAIL_ARGS;

    return NO_ERROR;
}

/*
 *  usage
 *      exename:  the name of the executable from argv[0]
 *
 *  Prints this programs expected usage
 *
 *  returns:    nothing, this is a void function
 *
 *  console:  This function prints the usage information
 *
 */
void usage(char *exename)
{
    printf("usage: %s -[h|a|c|d|f|p|z] options.  Where:\n", exename);
    printf("\t-h:  prints help\n");
    printf("\t-a id first_name last_name gpa(as 3 digit int):  adds a student\n");
    printf("\t-c:  counts the records in the database\n");
    printf("\t-d id:  deletes a student\n");
    printf("\t-f id:  finds and prints a student in the database\n");
    printf("\t-p:  prints all records in the student database\n");
    printf("\t-x:  compress the database file [EXTRA CREDIT]\n");
    printf("\t-z:  zero db file (remove all records)\n");
}

// Welcome to main()
int main(int argc, char *argv[])
{
    char opt;
    int fd;
    int rc;
    int exit_code;
    int id;
    int gpa;
    student_t student = {0};

    if ((argc < 2) || (*argv[1] != '-'))
    {
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1] + 1);

    if (opt == 'h')
    {
        usage(argv[0]);
        exit(EXIT_OK);
    }

    fd = open_db(DB_FILE, false);
    if (fd < 0)
    {
        exit(EXIT_FAIL_DB);
    }

    exit_code = EXIT_OK;
    switch (opt)
    {
    case 'a':
        if (argc != 6)
        {
            usage(argv[0]);
            exit_code = EXIT_FAIL_ARGS;
            break;
        }
        id = atoi(argv[2]);
        gpa = atoi(argv[5]);
        exit_code = validate_range(id, gpa);
        if (exit_code == EXIT_FAIL_ARGS)
        {
            printf(M_ERR_STD_RNG);
            break;
        }
        rc = add_student(fd, id, argv[3], argv[4], gpa);
        if (rc < 0)
            exit_code = EXIT_FAIL_DB;
        break;

    case 'c':
        rc = count_db_records(fd);
        if (rc < 0)
            exit_code = EXIT_FAIL_DB;
        break;

    case 'd':
        if (argc != 3)
        {
            usage(argv[0]);
            exit_code = EXIT_FAIL_ARGS;
            break;
        }
        id = atoi(argv[2]);
        rc = del_student(fd, id);
        if (rc < 0)
            exit_code = EXIT_FAIL_DB;
        break;

    case 'f':
        if (argc != 3)
        {
            usage(argv[0]);
            exit_code = EXIT_FAIL_ARGS;
            break;
        }
        id = atoi(argv[2]);
        rc = get_student(fd, id, &student);
        switch (rc)
        {
        case NO_ERROR:
            print_student(&student);
            break;
        case SRCH_NOT_FOUND:
            printf(M_STD_NOT_FND_MSG, id);
            exit_code = EXIT_FAIL_DB;
            break;
        default:
            printf(M_ERR_DB_READ);
            exit_code = EXIT_FAIL_DB;
            break;
        }
        break;

    case 'p':
        rc = print_db(fd);
        if (rc < 0)
            exit_code = EXIT_FAIL_DB;
        break;

    case 'x':
        fd = compress_db(fd);
        if (fd < 0)
            exit_code = EXIT_FAIL_DB;
        break;

    case 'z':
        close(fd);
        fd = open_db(DB_FILE, true);
        if (fd < 0)
        {
            exit_code = EXIT_FAIL_DB;
            break;
        }
        printf(M_DB_ZERO_OK);
        exit_code = EXIT_OK;
        break;

    default:
        usage(argv[0]);
        exit_code = EXIT_FAIL_ARGS;
    }

    close(fd);
    exit(exit_code);
}
