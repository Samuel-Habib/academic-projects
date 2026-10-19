package avengers;
/**
 * 
 * Using the Adjacency Matrix of n vertices and starting from Earth (vertex 0), 
 * modify the edge weights using the functionality values of the vertices that each edge 
 * connects, and then determine the minimum cost to reach Titan (vertex n-1) from Earth (vertex 0).
 * 
 * Steps to implement this class main method:
 * 
 * Step 1:
 * LocateTitanInputFile name is passed through the command line as args[0]
 * Read from LocateTitanInputFile with the format:
 *    1. g (int): number of generators (vertices in the graph)
 *    2. g lines, each with 2 values, (int) generator number, (double) funcionality value
 *    3. g lines, each with g (int) edge values, referring to the energy cost to travel from 
 *       one generator to another 
 * Create an adjacency matrix for g generators.
 * 
 * Populate the adjacency matrix with edge values (the energy cost to travel from one 
 * generator to another).
 * 
 * Step 2:
 * Update the adjacency matrix to change EVERY edge weight (energy cost) by DIVIDING it 
 * by the functionality of BOTH vertices (generators) that the edge points to. Then, 
 * typecast this number to an integer (this is done to avoid precision errors). The result 
 * is an adjacency matrix representing the TOTAL COSTS to travel from one generator to another.
 * 
 * Step 3:
 * LocateTitanOutputFile name is passed through the command line as args[1]
 * Use Dijkstra’s Algorithm to find the path of minimum cost between Earth and Titan. 
 * Output this number into your output file!
 * 
 * Note: use the StdIn/StdOut libraries to read/write from/to file.
 * 
 *   To read from a file use StdIn:
 *     StdIn.setFile(inputfilename);
 *     StdIn.readInt();
 *     StdIn.readDouble();
 * 
 *   To write to a file use StdOut (here, minCost represents the minimum cost to 
 *   travel from Earth to Titan):
 *     StdOut.setFile(outputfilename);
 *     StdOut.print(minCost);
 *  
 * Compiling and executing:
 *    1. Make sure you are in the ../InfinityWar directory
 *    2. javac -d bin src/avengers/*.java
 *    3. java -cp bin avengers/LocateTitan locatetitan.in locatetitan.out
 * 
 * @author Yashas Ravi
 * 
 */

 public class LocateTitan {
    public static int minCost(int[] lowestCost, boolean[] visited) {

        int low = Integer.MAX_VALUE;
        int val = -1;
        for (int i = 0; i < lowestCost.length; i++) {
            if (!visited[i] && lowestCost[i] <= low) {
                low = lowestCost[i];
                val = i;
            }
        }
        return val;
    }
 
 
    public static void main(String[] args) {
        if (args.length < 2) {
            StdOut.println("Execute: java LocateTitan <INput file> <OUTput file>");
            return;
        }
 
        String LocateTitanInputFile = args[0];
        StdIn.setFile(LocateTitanInputFile);
        int total = StdIn.readInt();
 
        double[] func = new double[total];
        for (int i = 0; i < total; i++) {
            StdIn.readInt();
            func[i] = StdIn.readDouble();
        }

        int[][] adjMatrix = new int[total][total];
        for (int i = 0; i < total; i++) {
            for (int j = 0; j < total; j++) {
                adjMatrix[i][j] = StdIn.readInt();
            }
        }
 
        for (int i = 0; i < total; i++) {
            for (int j = 0; j < total; j++) {
                if (adjMatrix[i][j] != 0) {
                    double w = func[i] * func[j];
                    adjMatrix[i][j] = (int) (adjMatrix[i][j] / w);
                }
            }
        }
 
        int[] lowestCost = new int[total];
        boolean[] visited = new boolean[total];
 
 
        for (int i = 0; i < total; i++) {
            lowestCost[i] = Integer.MAX_VALUE;
            visited[i] = false;
        }
 
 
        lowestCost[0] = 0;
 
        for (int i = 0; i < total - 1; i++) {
            int x = minCost(lowestCost, visited);
            visited[x] = true;
            for (int y = 0; y < total; y++) {
                if (!visited[y] && adjMatrix[x][y] != 0 && lowestCost[x] != Integer.MAX_VALUE && lowestCost[x] + adjMatrix[x][y] < lowestCost[y]) {
                    lowestCost[y] = lowestCost[x] + adjMatrix[x][y];
                }
            }

        }

        int a1 = 0;
        while(a1 > 0){

        }
 
        String LocateTitanOuputFile = args[1];
        StdOut.setFile(LocateTitanOuputFile);
        StdOut.println(lowestCost[total - 1]);
    }
 }
 
