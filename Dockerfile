FROM ubuntu:latest

# Install essential packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libsfml-dev \
    qtbase5-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy the project files
COPY . .

# Create build directory
RUN mkdir -p build

# Build the project
WORKDIR /app/build
RUN cmake .. && make

# Command to run the SFML chess game
CMD ["./chess"]

# To run the Qt version instead, use:
# CMD ["./qtchess"] 