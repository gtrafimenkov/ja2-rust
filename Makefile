CLANG_FORMATTER ?= clang-format-13

format:
	find ja2 \( -iname '*.c' -o -iname '*.cc' -o -iname '*.cpp' -o -iname '*.h' \) \
		| xargs $(CLANG_FORMATTER) -i --style=file

format-modified:
	git status --porcelain | egrep -e '[.](c|cc|cpp|h)$$' | awk '{print $$2}' \
		| xargs $(CLANG_FORMATTER) -i --style=file
