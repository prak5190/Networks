all : 
	g++ a1.cpp -o a.out	
# Test cases with text 
	@echo 'Running with array - True test case'
	./a.out --array 20 --text "()()()()()"
	@echo 'Running with array - False test case'
	./a.out --array 20 --text "(((((((((("
	@echo 'Running with array - False test case'
	./a.out --array 20 --text "()(((()()()()"
	@echo 'Running with link - True test case'
	./a.out --link --text "()()()()()"
	@echo 'Running with link - False test case'
	./a.out --link  --text "()(((()()()()"
	@echo 'Running with link - False test case'
	./a.out --link --text "(((((((((("
# Test cases with file 
	@echo 'Running with array - File '
	./a.out --array 100 --file "test.txt"
	@echo 'Running with link - File'
	./a.out --link --file "test.txt"
