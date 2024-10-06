## To run the weather generator: 
`javac WeatherGenerator.java -Xlint:deprecation`
`java WeatherGenerator.java 1 2 3`
* Note: you need to supply WeatherGenerator.java with 3 values or else an error will occur

## 2048
### How to compile and execute 2048
* When in the Play2048 directory, type the following in the terminal 
`javac -d bin src/game/*.java -Xlint:deprecation`

* To execute the Animated Version, use `java -cp bin game.AnimatedDriver`
* To execute the Text Version, use `java -cp bin game.TextDriver`

### How to play 2048
* Once you've executed the program, press 2 to start the game
* **Use the WASD keys to make your moves**

## RUKINDERGARTEN
* RU Kindergarten is a simulation of a kindergarten through a few methods that involve the students standing in a line, students in their seats and a game of musical chairs through the use of singly linked lists, 2d arrays, and circularly linked list
### How to compile and execute RUKINDERGARTEN
* When in the RUKINDERGARTEN directory, type the following in the terminal
`javac -d bin src/kindergarten/*.java`
* To execute in the terminal, run `java -cp bin kindergarten.Driver`

## Huffman Coding
* This assignment is an implementation of the huffman coding algorithm using the tree data structure and allows data to be compressed into a much smaller size with no loss in data 

### How to compile and execute Huffman Coding
* `javac -d bin src/huffman/*.java`
* `java -cp bin huffman.Driver`


## InfinityWar

### How to compile and execute InfinityWar
* The program uses the context of the Infinity War movie to model different scenarios as graph problems, such as calculating the energy cost to travel through wormholes (representing the journey to Titan) and simulating Thanos’ snap on a network to see if the remaining graph is connected. Each method requires the use of graph algorithms like Dijkstra’s and manipulating adjacency matrices to solve these problems. 

* `javac -d bin src/avengers/*.java`
* `java -cp bin avengers.ForgeStormBreaker forgestormbreaker.in forgestormbreaker.out`
- this will compile using the ForgeStormBreaker main method while taking in forgestormbreaker.in and creating the file forgestormbreaker.out`