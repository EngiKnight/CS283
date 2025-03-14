#!/usr/bin/env bats
setup() {
  ./dsh -s -p 5678 &
  SERVER_PID=$!
  sleep 1
}
teardown() {
  kill $SERVER_PID 2>/dev/null || true
}
@test "remote shell echo command" {
  run bash -c 'printf "echo hello\nexit\n" | ./dsh -c -p 5678'
  [ "$status" -eq 0 ]
  [[ "$output" =~ "hello" ]]
}
@test "remote shell exit command" {
  run bash -c 'printf "exit\n" | ./dsh -c -p 5678'
  [ "$status" -eq 0 ]
}
@test "remote shell stop-server" {
  run bash -c 'printf "stop-server\n" | ./dsh -c -p 5678'
  [ "$status" -eq 0 ]
}
