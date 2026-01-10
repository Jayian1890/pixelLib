#!/bin/bash
# Setup script for clang-tidy integration
# This script helps install clang-tidy and sets up the environment

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Setting up clang-tidy for pixelLib project...${NC}"

# Detect OS
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo -e "${YELLOW}Detected macOS${NC}"
    
    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
        echo -e "${RED}Homebrew not found. Please install Homebrew first:${NC}"
        echo '/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"'
        exit 1
    fi
    
    # Check if LLVM is installed
    if ! brew list llvm &> /dev/null; then
        echo -e "${YELLOW}Installing LLVM (includes clang-tidy)...${NC}"
        brew install llvm
    else
        echo -e "${GREEN}LLVM is already installed${NC}"
    fi
    
    # Add LLVM to PATH if not already there
    if [[ ":$PATH:" != *":/usr/local/opt/llvm/bin:"* ]]; then
        echo -e "${YELLOW}Adding LLVM to PATH...${NC}"
        echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >> ~/.zshrc
        echo -e "${YELLOW}Please run: source ~/.zshrc${NC}"
    fi
    
    # Set environment variables
    export PATH="/usr/local/opt/llvm/bin:$PATH"
    
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo -e "${YELLOW}Detected Linux${NC}"
    
    # Try apt-get (Ubuntu/Debian)
    if command -v apt-get &> /dev/null; then
        echo -e "${YELLOW}Installing clang-tidy via apt...${NC}"
        sudo apt-get update
        sudo apt-get install -y clang-tidy
    # Try yum (RHEL/CentOS)
    elif command -v yum &> /dev/null; then
        echo -e "${YELLOW}Installing clang-tidy via yum...${NC}"
        sudo yum install -y clang-tidy
    # Try dnf (Fedora)
    elif command -v dnf &> /dev/null; then
        echo -e "${YELLOW}Installing clang-tidy via dnf...${NC}"
        sudo dnf install -y clang-tidy
    else
        echo -e "${RED}Unsupported package manager. Please install clang-tidy manually.${NC}"
        exit 1
    fi
else
    echo -e "${RED}Unsupported OS: $OSTYPE${NC}"
    echo "Please install clang-tidy manually and add it to your PATH"
    exit 1
fi

# Verify installation
if command -v clang-tidy &> /dev/null; then
    echo -e "${GREEN}✓ clang-tidy found at: $(which clang-tidy)${NC}"
    echo -e "${GREEN}✓ Version: $(clang-tidy --version | head -n1)${NC}"
else
    echo -e "${RED}✗ clang-tidy installation failed${NC}"
    exit 1
fi

# Test configuration
echo -e "${BLUE}Testing clang-tidy configuration...${NC}"
if [[ -f ".clang-tidy" ]]; then
    echo -e "${GREEN}✓ .clang-tidy configuration found${NC}"
else
    echo -e "${RED}✗ .clang-tidy configuration not found${NC}"
    exit 1
fi

# Create build directory if it doesn't exist
mkdir -p build

# Test on a sample file
echo -e "${BLUE}Testing clang-tidy on sample file...${NC}"
if clang-tidy --checks="modernize-use-nullptr" tests/test_main.cc -- -std=c++23 -Iinclude -Itests -Ithird-party &> /dev/null; then
    echo -e "${GREEN}✓ clang-tidy test passed${NC}"
else
    echo -e "${YELLOW}clang-tidy test had issues (this is normal if there are warnings)${NC}"
fi

echo -e "${GREEN}Setup complete!${NC}"
echo -e "${BLUE}You can now use:${NC}"
echo "  make clang-tidy          # Run all checks"
echo "  make clang-tidy-fix      # Apply fixes"
echo "  make clang-tidy-report   # Generate report"
echo ""
echo -e "${BLUE}Or use the script directly:${NC}"
echo "  ./tools/run-clang-tidy.sh --help"
