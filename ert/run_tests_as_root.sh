# Runs all tests, including those requiring root
# Beware, this will create build artifacts owned by root
# It's recommended to provision man limits.conf(5) as in the conf directory
sudo env "PATH=$PATH" cargo test --features privelaged_tests
