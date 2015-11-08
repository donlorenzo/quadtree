CC=gcc
DEBUG_CFLAGS=-Wall -Werror -fPIC -g -O0 -fno-omit-frame-pointer -std=c90
RELEASE_CFLAGS=-Wall -Werror -fPIC -O2 -fomit-frame-pointer -std=c90
CFLAGS=$(DEBUG_CFLAGS)
LDFLAGS=-shared
LIBDIRS=
BUILD_DIR=build
SRC_DIR=src
TEST_DIR=test
INSTALL_LIB_DIR=/usr/local/lib
INSTALL_HEADER_DIR=/usr/local/include/quadtree
FILES=quadtree.c utils.c
SRC=$(addprefix $(SRC_DIR)/,$(FILES))
OBJ=$(addprefix $(BUILD_DIR)/,$(FILES:%.c=%.o))
TARGET=libquadtree.so

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBDIRS) $(OBJ) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h $(SRC_DIR)/quadtree.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

$(INSTALL_LIB_DIR):
	mkdir -p $@

$(INSTALL_HEADER_DIR):
	mkdir -p $@

quadtree_test: $(TEST_DIR)/test.c $(TARGET)
	$(CC) $< $(CFLAGS) -lquadtree -L. -Isrc  -o $@

install: $(TARGET) $(INSTALL_LIB_DIR) $(INSTALL_HEADER_DIR)
	cp -a $(TARGET) $(INSTALL_LIB_DIR)/$(TARGET)
	cp -a $(SRC_DIR)/quadtree.h  $(INSTALL_HEADER_DIR)/quadtree.h

.PHONY: clean docs
clean:
	rm -rf $(BUILD_DIR) $(TARGET) quadtree_test

docs: $(SRC_DIR)/quadtree.h
	cd docs; doxygen Doxyfile; cd ..
