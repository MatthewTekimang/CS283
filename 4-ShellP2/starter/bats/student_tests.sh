#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Check that the shell can exit with zero status" {
    run ./dsh <<< "exit"
    [ "$status" -eq 0 ]
}


@test "Check that we can create and cd into a directory" {
    tmpdir="/tmp/test_dir_$$"
    mkdir -p "$tmpdir"
    run ./dsh <<EOF
cd $tmpdir
pwd
EOF
    [[ "$output" == *"$tmpdir"* ]]
    [ "$status" -eq 0 ]
    rmdir "$tmpdir"
}

@test "Check that an unknown command triggers an error message" {
    run ./dsh <<< "this_command_does_not_exist"
    [[ "$output" == *"No such file or directory"* ]]
    [ "$status" -eq 0 ]
}

@test "Check that echo preserves quoted arguments" {
    run ./dsh <<EOF
echo "Hello   World!"
exit
EOF
    [[ "$output" == *"Hello   World!"* ]]
    [ "$status" -eq 0 ]
}

@test "Check cd with no arguments (should go to /tmp)" {
    run ./dsh <<EOF
cd
pwd
exit
EOF
    [[ "$output" == *"/tmp"* ]]
    [ "$status" -eq 0 ]
}


@test "Check a second cd usage to another directory" {
    tmp2="/tmp/test_dir_two_$$"
    mkdir -p "$tmp2"
    run ./dsh <<EOF
cd $tmp2
pwd
exit
EOF
    [[ "$output" == *"$tmp2"* ]]
    [ "$status" -eq 0 ]
    rmdir "$tmp2"
}

@test "Check exit command terminates the shell" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}