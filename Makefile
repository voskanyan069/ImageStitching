CC=g++
CFLAGS=-ggdb --std=c++17 -fPIC $(INCLUDES)
SRC_DIR=./src
INCLUDES=-I./inc/ -I/usr/local/include/opencv4/
BUILD_DIR=./build
OBJ_DIR=$(BUILD_DIR)/obj
LIBS=-L/usr/local/lib/ -lopencv_stitching -lopencv_highgui -lopencv_features2d -lopencv_imgcodecs -lopencv_imgproc -lopencv_calib3d -lopencv_flann -lopencv_core -lopencv_video -lboost_program_options -lboost_filesystem
MAIN_OBJ = $(OBJ_DIR)/main.o
SOURCES = $(wildcard $(addsuffix /*.cxx, $(SRC_DIR)))
TMP_OBJ   = $(patsubst %.cxx, %.o, $(SOURCES))
OBJECTS = $(subst $(SRC_DIR),$(OBJ_DIR), $(TMP_OBJ))
APP=$(BUILD_DIR)/image_stitching

build: $(APP) $(SRC_DIR)/*

.PHONY: run
run: $(APP)
	./$< -i res/v2/stitch_imgs -o 2

$(APP): $(OBJECTS)
	@echo $(OBJECTS)
	$(CC) -o $@ $(CFLAGS) $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cxx
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
