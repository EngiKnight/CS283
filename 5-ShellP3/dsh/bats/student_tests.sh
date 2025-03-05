#!/usr/bin/env bats

setup() {
  rm -f out.txt
}

teardown() {
  rm -f out.txt
}

@test "external command execution" {
  run bash -c 'printf "echo hello\nexit\n" | ./dsh'
  [ "$status" -eq 0 ]
  [[ "$output" =~ "hello" ]]
}

@test "piped command execution" {
  run bash -c 'printf "echo hello | tr a-z A-Z\nexit\n" | ./dsh'
  [ "$status" -eq 0 ]
  [[ "$output" =~ "HELLO" ]]
}

@test "redirection >" {
  run bash -c 'printf "echo hello, class > out.txt\nexit\n" | ./dsh'
  run cat out.txt
  [ "$status" -eq 0 ]
  [ "$output" = "hello, class" ]
}

@test "redirection >> append" {
  run bash -c 'printf "echo first > out.txt\necho second >> out.txt\nexit\n" | ./dsh'
  run cat out.txt
  [ "$status" -eq 0 ]
  [[ "$output" =~ "first" ]]
  [[ "$output" =~ "second" ]]
}

@test "built-in cd" {
  run bash -c 'mkdir -p testdir && printf "cd testdir\npwd\nexit\n" | ./dsh'
  [ "$status" -eq 0 ]
  [[ "$output" =~ "testdir" ]]
  rm -rf testdir
}

@test "built-in rc for command not found" {
  run bash -c 'printf "nonexistentcmd\nrc\nexit\n" | ./dsh'
  [ "$status" -eq 0 ]
  [[ "$output" =~ "Command not found in PATH" ]]
  [[ "$output" =~ "2" ]]
}
