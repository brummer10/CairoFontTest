PASS := features 

.PHONY : all

all:
	@$(CC) -g cairo_font_test.c  -lX11 `pkg-config --cflags --libs cairo` -o cairo_font_test
	@mkdir -p ./bin/
	@cp cairo_font_test ./bin/

clean:
	@rm -f cairo_font_test
	@rm -rf ./bin/

features: all
