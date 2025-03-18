#!/usr/bin/env bats
# File: student_tests.sh

TEST_PORT=1234

clean_output() {
  echo "$1" | sed '/^socket client mode:/d' | sed '/^socket server mode:/d' | sed '/^->/d'
}

setup_remote_server() {
    ./dsh -s -p "$TEST_PORT" &
    SERVER_PID=$!
    sleep 1
}

teardown_remote_server() {
    if kill -0 "$SERVER_PID" 2>/dev/null; then
        kill "$SERVER_PID"
        wait "$SERVER_PID" 2>/dev/null
    fi
}

@test "Check ls runs without errors" {
    run ./dsh <<EOF
ls
EOF
    [ "$status" -eq 0 ]
}

@test "Local mode: piped command execution (echo and tr)" {
    run ./dsh <<EOF
echo hello world | tr a-z A-Z
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="HELLOWORLD"
    echo "$stripped_output" | grep -q "$expected_output"
    [ "$status" -eq 0 ]
}

@test "multiple piped commands (echo, tr, rev)" {
    run ./dsh <<EOF
echo hello world | tr a-z A-Z | rev
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="DLROWOLLEH"
    echo "$stripped_output" | grep -q "$expected_output"
    [ "$status" -eq 0 ]
}

@test "Built-in cd command changes directory" {
    run ./dsh <<EOF
cd /
pwd
EOF
    echo "$output" | grep -q "^/"
    [ "$status" -eq 0 ]
}

@test "echo command outputs expected text" {
  run bash -c 'echo "echo hello world" | ./dsh'
  [ "$status" -eq 0 ]
  [[ "$output" =~ "hello world" ]]
}

@test "uname -a command outputs Linux" {
  run bash -c 'echo "uname" | ./dsh'
  [ "$status" -eq 0 ]
  [[ "$output" =~ "Linux" ]]
}

@test "cd command changes directory" {
    run bash -c 'echo "pwd; cd ..; pwd" | ./dsh'
    [ "$status" -eq 0 ]
    oldpwd=$(echo "$output" | sed -n '1p')
    newpwd=$(echo "$output" | sed -n '2p')
    [ "$oldpwd" != "$newpwd" ]
}

@test "ls command lists expected file" {
  run bash -c 'echo "ls" | ./dsh'
  [ "$status" -eq 0 ]
  [[ "$output" =~ "dsh" ]]
}

@test "Test pipeline: echo hello | wc -c" {
    run ./dsh <<EOF
echo hello | wc -c
EOF
    [ "$status" -eq 0 ]
    [ "${lines[0]}" -eq 6 ] 
}

@test "Test input redirection: wc -l < file.txt" {
    echo -e "line1\nline2\nline3" > file.txt
    run ./dsh <<EOF
wc -l < file.txt
EOF
    [ "$status" -eq 0 ]
    [ "${lines[0]}" -eq 3 ]
    rm file.txt
}

@test "Test output redirection: echo hello > out.txt" {
    run ./dsh <<EOF
echo hello > out.txt
EOF
    [ "$status" -eq 0 ]
    [ "$(cat out.txt)" = "hello" ]  
    rm out.txt
}

@test "Test append output redirection: echo line1 >> out.txt and echo line2 >> out.txt" {
    echo "line1" > out.txt
    run ./dsh <<EOF
echo line2 >> out.txt
EOF
    [ "$status" -eq 0 ]
    [ "$(cat out.txt)" = $'line1\nline2' ] 
    rm out.txt
}

@test "Test combined input and output redirection: wc -l < file.txt > out.txt" {
    echo -e "line1\nline2\nline3" > file.txt
    run ./dsh <<EOF
wc -l < file.txt > out.txt
EOF
    [ "$status" -eq 0 ]
    [ "$(cat out.txt)" -eq 3 ] 
    rm file.txt out.txt
}

## Helper Functions for Remote Server Testing
start_server() {
    local PORT=$((8000 + RANDOM % 1000))
    local MAX_ATTEMPTS=5
    local ATTEMPTS=0
    
    while [ $ATTEMPTS -lt $MAX_ATTEMPTS ]; do
        if nc -z 127.0.0.1 $PORT 2>/dev/null; then
            PORT=$((8000 + RANDOM % 1000))
            ATTEMPTS=$((ATTEMPTS+1))
            continue
        fi
        
        ./dsh -s -p $PORT > server_log.txt 2>&1 &
        local SERVER_PID=$!
        
        sleep 0.5
        
        if ! nc -z 127.0.0.1 $PORT 2>/dev/null; then
            kill $SERVER_PID 2>/dev/null || true
            PORT=$((8000 + RANDOM % 1000))
            ATTEMPTS=$((ATTEMPTS+1))
            continue
        fi
        
        echo "$PORT:$SERVER_PID"
        return 0
    done
    
    echo "Failed to start server after $MAX_ATTEMPTS attempts" >&2
    return 1
}

@test "Remote: echo command via client-server" {
    SERVER_INFO=$(start_server)
    [ "$?" -eq 0 ] || fail "Failed to start server"
    
    PORT=$(echo $SERVER_INFO | cut -d':' -f1)
    SERVER_PID=$(echo $SERVER_INFO | cut -d':' -f2)
    
    run bash -c "echo 'echo hello' | ./dsh -c -i 127.0.0.1 -p $PORT"
    
    kill $SERVER_PID 2>/dev/null || true
    wait $SERVER_PID 2>/dev/null || true
    
    [ "$status" -eq 0 ] || { cat server_log.txt; fail "Command failed with status $status: $output"; }
    [[ "$output" =~ "hello" ]] || { cat server_log.txt; fail "Expected 'hello' in output, got: $output"; }
}

@test "Remote: cd command changes directory in session" {
    SERVER_INFO=$(start_server)
    [ "$?" -eq 0 ] || fail "Failed to start server"
    
    PORT=$(echo $SERVER_INFO | cut -d':' -f1)
    SERVER_PID=$(echo $SERVER_INFO | cut -d':' -f2)
    
    initial_pwd=$(echo "pwd" | ./dsh -c -i 127.0.0.1 -p $PORT)
    
    run bash -c "echo 'cd .. ; pwd' | ./dsh -c -i 127.0.0.1 -p $PORT"
    
    kill $SERVER_PID 2>/dev/null || true
    wait $SERVER_PID 2>/dev/null || true
    
    [ "$status" -eq 0 ] || { cat server_log.txt; fail "Command failed with status $status"; }
    new_pwd=$(echo "$output" | tail -n1)
    [ "$new_pwd" != "$initial_pwd" ] || { cat server_log.txt; fail "Directory did not change"; }
}

@test "Remote: ls command lists files via client" {
    SERVER_INFO=$(start_server)
    [ "$?" -eq 0 ] || fail "Failed to start server"
    
    PORT=$(echo $SERVER_INFO | cut -d':' -f1)
    SERVER_PID=$(echo $SERVER_INFO | cut -d':' -f2)
    
    run bash -c "echo 'ls' | ./dsh -c -i 127.0.0.1 -p $PORT"
    
    kill $SERVER_PID 2>/dev/null || true
    wait $SERVER_PID 2>/dev/null || true
    
    [ "$status" -eq 0 ] || { cat server_log.txt; fail "Command failed with status $status"; }
    [[ "$output" =~ "dsh" ]] || { cat server_log.txt; fail "Expected 'dsh' in output: $output"; }
}

@test "Remote: rc returns exit code of previous command" {
    SERVER_INFO=$(start_server)
    [ "$?" -eq 0 ] || fail "Failed to start server"
    
    PORT=$(echo $SERVER_INFO | cut -d':' -f1)
    SERVER_PID=$(echo $SERVER_INFO | cut -d':' -f2)
    
    echo "invalidcommand" | ./dsh -c -i 127.0.0.1 -p $PORT
    run bash -c "echo 'rc' | ./dsh -c -i 127.0.0.1 -p $PORT"
    
    kill $SERVER_PID 2>/dev/null || true
    wait $SERVER_PID 2>/dev/null || true
    
    [ "$status" -eq 0 ] || { cat server_log.txt; fail "Command failed with status $status"; }
    [[ "$output" =~ "127" ]] || { cat server_log.txt; fail "Expected exit code in output: $output"; }
}

@test "Remote: exit command disconnects client" {
    SERVER_INFO=$(start_server)
    [ "$?" -eq 0 ] || fail "Failed to start server"
    
    PORT=$(echo $SERVER_INFO | cut -d':' -f1)
    SERVER_PID=$(echo $SERVER_INFO | cut -d':' -f2)
    
    run bash -c "echo 'exit' | ./dsh -c -i 127.0.0.1 -p $PORT"
    [ "$status" -eq 0 ] || { cat server_log.txt; fail "Exit command failed with status $status"; }
    
    run bash -c "echo 'echo still alive' | ./dsh -c -i 127.0.0.1 -p $PORT"
    
    kill $SERVER_PID 2>/dev/null || true
    wait $SERVER_PID 2>/dev/null || true
    
    [ "$status" -eq 0 ] || { cat server_log.txt; fail "Second connection failed with status $status"; }
    [[ "$output" =~ "still alive" ]] || { cat server_log.txt; fail "Expected 'still alive' in output: $output"; }
}

@test "Remote: multiple client connections work" {
    SERVER_INFO=$(start_server)
    [ "$?" -eq 0 ] || fail "Failed to start server"
    
    PORT=$(echo $SERVER_INFO | cut -d':' -f1)
    SERVER_PID=$(echo $SERVER_INFO | cut -d':' -f2)
    
    run bash -c "echo 'echo client one' | ./dsh -c -i 127.0.0.1 -p $PORT"
    [ "$status" -eq 0 ] || { cat server_log.txt; fail "First client connection failed with status $status"; }
    [[ "$output" =~ "client one" ]] || { cat server_log.txt; fail "Expected 'client one' in output: $output"; }
    
    run bash -c "echo 'echo client two' | ./dsh -c -i 127.0.0.1 -p $PORT"
    
    kill $SERVER_PID 2>/dev/null || true
    wait $SERVER_PID 2>/dev/null || true

    [ "$status" -eq 0 ] || { cat server_log.txt; fail "Second client connection failed with status $status"; }
    [[ "$output" =~ "client two" ]] || { cat server_log.txt; fail "Expected 'client two' in output: $output"; }
}