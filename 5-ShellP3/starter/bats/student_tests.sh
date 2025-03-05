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

@test "Check that an unknown command triggers error message" {
    run ./dsh <<< "nonexistentcommand"
    [[ "$output" == *"not found"* ]]
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

@test "Test simple pipe: ls | grep" {
    run ./dsh <<EOF
ls | grep ".c"
exit
EOF
    [[ "$output" == *".c"* ]]
    [ "$status" -eq 0 ]
}

@test "Test pipe with three commands" {
    run ./dsh <<EOF
echo "one two three" | grep "one" | cat
exit
EOF
    [[ "$output" == *"one"* ]]
    [ "$status" -eq 0 ]
}

@test "Test cd to non-existent directory" {
    run ./dsh <<EOF
cd /nonexistentdirectory
exit
EOF
    [[ "$output" == *"No such file or directory"* ]]
    [ "$status" -eq 0 ]
}

@test "Test multiple cd commands" {
    run ./dsh <<EOF
cd /tmp
mkdir -p testdir
cd testdir
pwd
cd ..
rmdir testdir
exit
EOF
    [[ "$output" == *"/tmp/testdir"* ]]
    [ "$status" -eq 0 ]
}

@test "Test echo with many spaces" {
    run ./dsh <<EOF
echo "This     has     many    spaces"
exit
EOF
    [[ "$output" == *"This     has     many    spaces"* ]]
    [ "$status" -eq 0 ]
}

@test "Test cat on non-existent file" {
    run ./dsh <<EOF
cat nonexistentfile
exit
EOF
    [[ "$output" == *"No such file or directory"* ]]
    [ "$status" -eq 0 ]
}

@test "Test grep with a pipe" {
    run ./dsh <<EOF
echo "apple banana cherry" | grep "banana"
exit
EOF
    [[ "$output" == *"banana"* ]]
    [ "$status" -eq 0 ]
}

@test "Test pwd after cd" {
    run ./dsh <<EOF
cd /tmp
pwd
exit
EOF
    [[ "$output" == *"/tmp"* ]]
    [ "$status" -eq 0 ]
}

@test "Test ls -l command" {
    run ./dsh <<EOF
ls -l
exit
EOF
    [[ "$output" == *"total"* ]]
    [ "$status" -eq 0 ]
}

@test "Test handling empty lines" {
    run ./dsh <<EOF

pwd

exit
EOF
    [[ "$output" == *"warning: no commands provided"* ]]
    [ "$status" -eq 0 ]
}

@test "Test non-existent command with arguments" {
    run ./dsh <<EOF
nonexistentcommand arg1 arg2
exit
EOF
    [[ "$output" == *"not found"* ]]
    [ "$status" -eq 0 ]
}

@test "Test echo with multiple arguments" {
    run ./dsh <<EOF
echo arg1 arg2 arg3
exit
EOF
    [[ "$output" == *"arg1 arg2 arg3"* ]]
    [ "$status" -eq 0 ]
}

@test "Test 'which' command" {
    run ./dsh <<EOF
which ls
exit
EOF
    [[ "$output" == *"/bin/ls"* || "$output" == *"/usr/bin/ls"* ]]
    [ "$status" -eq 0 ]
}

@test "Test pipe with invalid first command" {
    run ./dsh <<EOF
nonexistentcommand | grep test
exit
EOF
    [[ "$output" == *"not found"* ]]
    [ "$status" -eq 0 ]
}

@test "Test pipe with invalid second command" {
    run ./dsh <<EOF
echo test | nonexistentcommand
exit
EOF
    [[ "$output" == *"not found"* ]]
    [ "$status" -eq 0 ]
}

@test "Test pipe with spaces around |" {
    run ./dsh <<EOF
echo test   |   grep test
exit
EOF
    [[ "$output" == *"test"* ]]
    [ "$status" -eq 0 ]
}

@test "Test using absolute path command" {
    run ./dsh <<EOF
/bin/echo "test absolute path"
exit
EOF
    [[ "$output" == *"test absolute path"* ]]
    [ "$status" -eq 0 ]
}

@test "Test quoted pipe character" {
    run ./dsh <<EOF
echo "This | is not a pipe"
exit
EOF
    [[ "$output" == *"This | is not a pipe"* ]]
    [ "$status" -eq 0 ]
}

@test "Test multi-pipe with more than 3 commands" {
    run ./dsh <<EOF
echo "test multi-pipe" | cat | grep test | cat
exit
EOF
    [[ "$output" == *"test multi-pipe"* ]]
    [ "$status" -eq 0 ]
}

@test "Test redirect error handling" {
    run ./dsh <<EOF
ls | grep nonexistentfile | wc -l
exit
EOF
    [[ "$output" == *"0"* ]]
    [ "$status" -eq 0 ]
}

@test "Test return code from last command" {
    run ./dsh <<EOF
false
rc
exit
EOF
    [[ "$output" == *"1"* ]]
    [ "$status" -eq 0 ]
}

@test "Test return code from successful command" {
    run ./dsh <<EOF
true
rc
exit
EOF
    [[ "$output" == *"0"* ]]
    [ "$status" -eq 0 ]
}

@test "Test handling of too many pipe commands" {
    run ./dsh <<EOF
echo test | cat | cat | cat | cat | cat | cat | cat | cat | cat
exit
EOF
    [[ "$output" == *"error: piping limited to"* ]]
    [ "$status" -eq 0 ]
}