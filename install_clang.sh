# Initially, we used g++, but later on figured out that some of the stuff we
# implemented required C++20 features that required a newer version of g++ that
# not all team members could download.

# Clang 16 seems to be available on more distros and to support the features we
# need.

wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 16
sudo apt install clang-16 libc++-16-dev libc++abi-16-dev
