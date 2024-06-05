#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check and install valgrind if not present
install_valgrind() {
    if ! command -v valgrind &> /dev/null; then
        echo -e "${BLUE}Valgrind not found. Installing valgrind...${NC}"
        if [[ -f /etc/os-release ]]; then
            . /etc/os-release
            if [[ "$ID" == "ubuntu" ]]; then
                sudo apt-get update && sudo apt-get install -y valgrind
            elif [[ "$ID" == "debian" ]]; then
                sudo apt-get update && sudo apt-get install -y valgrind
            else
                echo -e "${RED}Unsupported Linux distribution for automatic valgrind installation.${NC}"
                exit 1
            fi
        elif [[ "$OSTYPE" == "darwin"* ]]; then
            brew install valgrind
        else
            echo -e "${RED}Unsupported OS for automatic valgrind installation.${NC}"
            exit 1
        fi
    else
        echo -e "${GREEN}Valgrind is already installed.${NC}"
    fi
}

# Install Homebrew on macOS if not present
install_brew() {
    if [[ "$OSTYPE" == "darwin"* ]] && ! command -v brew &> /dev/null; then
        echo -e "${BLUE}Homebrew not found. Installing Homebrew...${NC}"
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
}

# Install necessary packages
install_dependencies() {
    if [[ -f /etc/os-release ]]; then
        . /etc/os-release
        if [[ "$ID" == "ubuntu" ]]; then
            sudo apt-get update
            sudo apt-get install -y build-essential python3 python3-pip
        elif [[ "$ID" == "debian" ]]; then
            sudo apt-get update
            sudo apt-get install -y build-essential python3 python3-pip
        else
            echo -e "${RED}Unsupported Linux distribution for automatic dependency installation.${NC}"
            exit 1
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        install_brew
        brew update
        brew install python
    else
        echo -e "${RED}Unsupported OS for automatic dependency installation.${NC}"
        exit 1
    fi
}


# Paths to the symnmf.py files
current_dir="."
prev_dir="Prev_final_100"
current_symnmf_path="${current_dir}/symnmf.py"
prev_symnmf_path="${prev_dir}/symnmf.py"

# Paths to the symnmf.c files
current_symnmf_c_path="${current_dir}/symnmf"
prev_symnmf_c_path="${prev_dir}/symnmf"

# Function to generate input files with points
generate_input_file() {
    local file_name=$1
    local num_points=$2
    local dimensions=$3

    echo -e "${BLUE}Generating ${YELLOW}$num_points${BLUE} points with ${YELLOW}$dimensions${BLUE} dimensions in ${YELLOW}$file_name${NC}"
    rm -f $file_name
    for ((i=0; i<num_points; i++)); do
        point=""
        for ((j=0; j<dimensions; j++)); do
            point+=$(echo "scale=2; $RANDOM/32768" | bc -l)
            if [ $j -lt $((dimensions - 1)) ]; then
                point+=","
            fi
        done
        echo $point >> $file_name
    done
}

test_non_existing_file_C() {
    local symnmf_path=$1
    local goal=$2
    local file_name="non_existing_file.txt"
    local expected_error="An Error Has Occurred"
    local output=$($symnmf_path $goal $file_name 2>&1)

    if [[ "$output" == *"$expected_error"* ]]; then
        echo -e "${GREEN}Error handling test for non-existing file passed for $symnmf_path.${NC}"
    else
        echo -e "${RED}Error handling test for non-existing file failed for $symnmf_path.${NC}"
        echo -e "${BLUE}Expected error message was not found. Actual output:${NC}\n$output"
    fi

    # Run valgrind to check for memory leaks in C tests in non existing files
    echo -e "${BLUE}Running valgrind for current C test case: ${YELLOW}$goal $file_name${NC}"
    if run_valgrind_c_test $symnmf_path $goal $file_name; then
    echo -e "${GREEN}Valgrind check passed for current C test case '${YELLOW}$goal $file_name${GREEN}'.${NC}\n"
    else
        echo -e "${RED}Valgrind check failed for current C test case '${YELLOW}$goal $file_name${RED}'.${NC}"
    fi  
}

test_non_existing_file_python() {
    local symnmf_path=$1
    local goal=$2
    local k=$3
    local file_name="non_existing_file.txt"
    local expected_error="An Error Has Occurred"
    local output=$(python3 $symnmf_path $k $goal $file_name 2>&1)

    if [[ "$output" == *"$expected_error"* ]]; then
        echo -e "${GREEN}Error handling test for non-existing file passed for $symnmf_path.${NC}"
    else
        echo -e "${RED}Error handling test for non-existing file failed for $symnmf_path.${NC}"
        echo -e "${BLUE}Expected error message was not found. Actual output:${NC}\n$output"
    fi
}

# Expanded test cases (K, goal, file_name, num_points, dimensions)
test_cases=(
"2 sym test_input_small1.txt 5 2"
"2 ddg test_input_small2.txt 5 3"
"2 norm test_input_small3.txt 7 4"
"2 symnmf test_input_small4.txt 6 2"
"2 sym test_input_small5.txt 5 1"
"2 ddg test_input_small6.txt 5 1"
"2 norm test_input_small7.txt 7 1"
"2 symnmf test_input_small8.txt 6 1"
"2 symnmf test_input1.txt 5 3"
"3 ddg test_input2.txt 6 4"
"2 norm test_input3.txt 4 2"
"2 symnmf test_input4.txt 5 3"
"4 sym test_input5.txt 10 3"
"5 ddg test_input6.txt 12 5"
"3 norm test_input7.txt 8 4"
"3 symnmf test_input8.txt 7 3"
"6 sym test_input9.txt 15 5"
"4 ddg test_input10.txt 10 2"
"3 norm test_input11.txt 6 3"
"2 symnmf test_input12.txt 4 2"
"5 sym test_input13.txt 11 4"
"3 ddg test_input14.txt 7 3"
"4 norm test_input15.txt 9 4"
"6 symnmf test_input16.txt 14 5"
"2 sym test_input17.txt 5 2"
"3 ddg test_input18.txt 6 3"
"4 norm test_input19.txt 8 4"
"5 symnmf test_input20.txt 10 5"
"7 sym test_input21.txt 20 6"
"5 ddg test_input22.txt 15 5"
"4 norm test_input23.txt 10 4"
"3 symnmf test_input24.txt 8 3"
"8 sym test_input25.txt 25 7"
"6 ddg test_input26.txt 18 6"
"5 norm test_input27.txt 12 5"
"4 symnmf test_input28.txt 9 4"
"9 sym test_input29.txt 30 8"
"7 ddg test_input30.txt 21 7"
"6 norm test_input31.txt 14 6"
"5 symnmf test_input32.txt 11 5"
"10 sym test_input33.txt 35 9"
"8 ddg test_input34.txt 24 8"
"7 norm test_input35.txt 16 7"
"6 symnmf test_input36.txt 13 6"
)

# Function to run Python test and capture output
run_python_test() {
    local symnmf_path=$1
    local K=$2
    local goal=$3
    local file_name=$4
    python3 $symnmf_path $K $goal $file_name
}

# Function to run C test and capture output
run_c_test() {
    local symnmf_path=$1
    local goal=$2
    local file_name=$3
    $symnmf_path $goal $file_name
}

# Function to run C test with valgrind and capture output
# Function to run C test with valgrind and check for memory leaks
run_valgrind_c_test() {
    local symnmf_path=$1
    local goal=$2
    local file_name=$3
    local valgrind_output

    valgrind_output=$(valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 $symnmf_path $goal $file_name 2>&1)
    echo "$valgrind_output"

    if echo "$valgrind_output" | grep -q "ERROR SUMMARY: 0 errors"; then
        return 0
    else
        return 1
    fi
}
# Ensure dependencies are installed
install_dependencies
install_valgrind

# Compile the symnmf.py files
echo -e "${BLUE}Compiling current symnmf.py...${NC}"
(cd $current_dir && python3 setup.py build_ext --inplace)
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed for current symnmf.py${NC}"
    exit 1
fi

echo -e "${BLUE}Compiling previous symnmf.py...${NC}"
(cd $prev_dir && python3 setup.py build_ext --inplace)
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed for previous symnmf.py${NC}"
    exit 1
fi

# Compile the symnmf.c files using make
echo -e "${BLUE}Compiling current symnmf.c...${NC}"
(cd $current_dir && make)
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed for current symnmf.c${NC}"
    exit 1
fi

echo -e "${BLUE}Compiling previous symnmf.c...${NC}"
(cd $prev_dir && make)
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed for previous symnmf.c${NC}"
    exit 1
fi

# Iterate over test cases
for test_case in "${test_cases[@]}"; do
    read -r K goal file_name num_points dimensions <<< "$test_case"
    
    echo -e "\n${BLUE}Running test case: ${YELLOW}K=$K, goal=$goal, file_name=$file_name${NC}\n"
    
    # Generate input file
    generate_input_file $file_name $num_points $dimensions

    # Ensure K < num_points
    if [ $K -ge $num_points ]; then
        echo -e "${RED}Skipping test case: K ($K) must be less than the number of points ($num_points)${NC}\n"
        continue
    fi

    # Run Python tests and capture output to temporary files
    current_python_output=$(run_python_test $current_symnmf_path $K $goal $file_name)
    prev_python_output=$(run_python_test $prev_symnmf_path $K $goal $file_name)

    # Compare Python outputs
    if [ "$current_python_output" == "$prev_python_output" ]; then
        echo -e "${GREEN}Python test case '${YELLOW}$K $goal $file_name${GREEN}' passed.${NC}\n"
    else
        echo -e "${RED}Python test case '${YELLOW}$K $goal $file_name${RED}' failed.${NC}"
        echo -e "${BLUE}Current Python output:${NC}\n$current_python_output"
        echo -e "${BLUE}Previous Python output:${NC}\n$prev_python_output\n"
    fi

    # Run C tests only for goals sym, ddg, norm
    if [[ "$goal" == "sym" || "$goal" == "ddg" || "$goal" == "norm" ]]; then
        current_c_output=$(run_c_test $current_symnmf_c_path $goal $file_name)
        prev_c_output=$(run_c_test $prev_symnmf_c_path $goal $file_name)

        # Compare C outputs
        if [ "$current_c_output" == "$prev_c_output" ]; then
            echo -e "${GREEN}C test case '${YELLOW}$goal $file_name${GREEN}' passed.${NC}\n"
        else
            echo -e "${RED}C test case '${YELLOW}$goal $file_name${RED}' failed.${NC}"
            echo -e "${BLUE}Current C output:${NC}\n$current_c_output"
            echo -e "${BLUE}Previous C output:${NC}\n$prev_c_output\n"
        fi

        # Run valgrind to check for memory leaks in C tests
        echo -e "${BLUE}Running valgrind for current C test case: ${YELLOW}$goal $file_name${NC}"
        if run_valgrind_c_test $current_symnmf_c_path $goal $file_name; then
            echo -e "${GREEN}Valgrind check passed for current C test case '${YELLOW}$goal $file_name${GREEN}'.${NC}\n"
        else
            echo -e "${RED}Valgrind check failed for current C test case '${YELLOW}$goal $file_name${RED}'.${NC}"
        fi
    fi

    # Inform the user that analysis.py will be run
    echo -e "${BLUE}Running analysis.py for K=$K and input file $file_name...${NC}"

    # Run analysis.py in the current directory
    current_analysis_output=$(python3 ${current_dir}/analysis.py $K $file_name 2>/dev/null)
    
    # Run analysis.py in the previous directory
    prev_analysis_output=$(python3 ${prev_dir}/analysis.py $K $file_name 2>/dev/null)

    # Compare the outputs of analysis.py
    if [ "$current_analysis_output" == "$prev_analysis_output" ]; then
        echo -e "${GREEN}Analysis test case '${YELLOW}$K $file_name${GREEN}' passed.${NC}\n"
    else
        echo -e "${RED}Analysis test case '${YELLOW}$K $file_name${RED}' failed.${NC}"
        echo -e "${BLUE}Current analysis output:${NC}\n$current_analysis_output"
        echo -e "${BLUE}Previous analysis output:${NC}\n$prev_analysis_output\n"
    fi

    # Clean up input file
    rm -f $file_name
done

echo -e "${BLUE}Testing error handling for non-existing files...${NC}"
test_non_existing_file_python $current_symnmf_path 7 "sym"
test_non_existing_file_C $current_symnmf_c_path "sym"
test_non_existing_file_python $current_symnmf_path 7 "ddg"
test_non_existing_file_C $current_symnmf_c_path "ddg"
test_non_existing_file_python $current_symnmf_path 7 "norm"
test_non_existing_file_C $current_symnmf_c_path "norm"
test_non_existing_file_python $current_symnmf_path 7 "symnmf"



# Clean up build directories and .so files
echo -e "${BLUE}Cleaning up build directories and .so files...${NC}"
(cd $current_dir && make clean)
(cd $prev_dir && make clean)
rm -rf ${current_dir}/build ${prev_dir}/build
rm -f ${current_dir}/*.so ${prev_dir}/*.so
