all:
	g++ main.cpp -o main -g \
	`pkg-config --cflags --libs freetype2` \
	-lglfw -lpthread -lX11 -ldl -lXrandr -lGLEW -lGLU -lGL \
	-DGL_SILENCE_DEPRECATION -DGLM_ENABLE_EXPERIMENTAL -I.
