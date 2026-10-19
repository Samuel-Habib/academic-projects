# Data Structures & Algorithms in Java
Rutgers University coursework in trees, graphs, linked lists, and arrays.

| Project | Description | Concepts |
| --- | --- | --- |
| [Huffman Coding](HuffmanCoding/) | Lossless text compression using Huffman coding | Binary trees, frequency-based encoding |
| [Graph Algorithms — InfinityWar](InfinityWar/) | Graph problems involving travel costs and connectivity | Dijkstra's algorithm, adjacency matrices, graph traversal |
| [RU Kindergarten](RUKINDERGARTEN/) | Student seating, line management, and musical-chairs simulation | Singly linked lists, circular linked lists, two-dimensional arrays |
| [2048](Play2048/) | Tile-based game with text and animated interfaces | Two-dimensional arrays, board updates, game logic |

## Run
Commands assume you are inside the corresponding project directory.

### Huffman Coding
```sh
javac -d bin src/huffman/*.java
java -cp bin huffman.Driver
```

### Graph Algorithms
```sh
javac -d bin src/avengers/*.java
java -cp bin avengers.ForgeStormBreaker forgestormbreaker.in forgestormbreaker.out
```

### RU Kindergarten
```sh
javac -d bin src/kindergarten/*.java
java -cp bin kindergarten.Driver
```

### 2048
```sh
javac -d bin src/game/*.java -Xlint:deprecation
java -cp bin game.AnimatedDriver
```
Use `game.TextDriver` for the text interface. Press 2 to start, then use WASD to move.
