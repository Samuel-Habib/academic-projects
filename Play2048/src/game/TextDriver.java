package game;
import java.util.*;
import java.awt.*;
import java.io.File;
import game.StdDraw;
import game.StdOut;
import game.StdIn;
import game.StdRandom;


import java.util.*;
/**
 * Text driver that enables students to test boards using the terminal.
 * @author Kal Pandit
 * @author Ishaan Ivaturi
 */
public class TextDriver {
    /**
     * Enables students to test commands interactively; calls methods to obtain
     * methods and play the game.
     * 
     **/
    public static void main(String[] args) {
        int action = getActionChoice();
        switch (action) {
            case 1: // Individual methods
            testIndividualMethods();
            break;
            case 2: // Play full game
            playFullGame();
            break;
            default:
            StdOut.println("Not a valid option!");
            break;
        }

        System.exit(0);
    }

    /**
     * Tests individual methods.
     * @param filename the file name to use for testing
     */
    private static void testIndividualMethods() {
        String[] methods = { "updateOpenSpaces", "addRandomTile", "swipeLeft", "mergeLeft", "transpose", "flipRows", "makeMove"};
        String[] options = { "Test a new input file", "Test another method on the same input file", "Quit" };
        int controlChoice = 1;
        do {
            StdOut.print("Enter a board input file name => ");
			String filename = StdIn.readLine();
            do {
                int method = getMethodChoice(methods);
                int[][] boardArray = readBoard(filename);
                Board board = new Board(boardArray);
                StdOut.println("Base Board:");
                board.print();
                testMethod(method, board);
                controlChoice = getControlChoice(options);
             } while (controlChoice == 2);
        } while (controlChoice == 1);

    }

    /**
     * Reads board from input file and returns board array.
     * @param filename the file name of the board to read
     * @return the board array
     */
    private static int[][] readBoard(String filename) {
        StdIn.setFile(filename);
        int[][] boardArray = new int[4][4];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                boardArray[i][j] = StdIn.readInt();
            }
        }
        StdIn.resetFile();
        return boardArray;
    }
    /**
     * Provides options for testing a new method or quitting.
     * @param options the possible options for testing
     * @return the result of this action
     */
    private static int getControlChoice(String[] options) {
        StdOut.println("What would you like to do?");
        for (int i = 0; i < 3; i++) {
            StdOut.println(i + 1 + ". " + options[i]);
        }
        StdOut.print("Enter a number => ");
        int number = Integer.parseInt(StdIn.readLine());
        return number;
    }
    /**
     * Provides options for testing methods or actions
     * @param options the possible options for methods or actions
     * @return the result of this action
     */
    private static int getMethodChoice(String[] methods) {
        StdOut.println("What method would you like to test?");
        for (int i = 0; i < 7; i++) {
            StdOut.println(i + 1 + ". " + methods[i]);
        }
        StdOut.print("Enter a number => ");
        int number = Integer.parseInt(StdIn.readLine());
        return number;
    }
    /**
     * Provides options for testing individual methods or the full game
     * @param options the possible options for methods or actions
     * @return the result of this action
     */
    private static int getActionChoice() {
        StdOut.println("What would you like to do?");
        StdOut.println("1. Test individual methods");
        StdOut.println("2. Play full game");

        StdOut.print("Enter a number => ");
        int number = Integer.parseInt(StdIn.readLine());
        return number;
    }
/**
 * Tests a method given by the user given their board
 * @param methodNumber the method number identified through the method list
 * @param board the student board
 */
    private static void testMethod(int methodNumber, Board board) {

        switch (methodNumber) {
            case 1:
                board.updateOpenSpaces();
                StdOut.println("Open Spaces: (** on board)");
                board.printOpenSpaces();
                break;

            case 2:
                StdRandom.setSeed(2022);
                board.updateOpenSpaces();
                board.addRandomTile();
                StdOut.println("New Board:");
                board.print();
                break;

            case 3:
                board.swipeLeft();
                StdOut.println("New Board:");
                board.print();
                break;

            case 4:
                board.mergeLeft();
                StdOut.println("New Board:");
                board.print();
                break;

            case 5:
                board.transpose();
                StdOut.println("New Board:");
                board.print();
                break;

            case 6:
                board.flipRows();
                StdOut.println("New Board:");
                board.print();
                break;
            case 7:
                StdOut.println("Actions: w = up, a = left, s = down, d = right");
                HashMap<Character, String> keyMap = new HashMap<>() {
                 {
                    put('w', "U");
                    put('a', "L");
                    put('s', "D");
                    put('d', "R");
                    put('W', "U");
                    put('A', "L");
                    put('S', "D");
                    put('D', "R");
                }
                };
                StdOut.print("Enter an action => ");
                String toRead = StdIn.readLine();
                String action = keyMap.getOrDefault(toRead.charAt(0), "Invalid");
                if (!action.equals("Invalid")) {
                    board.makeMove(action.charAt(0));
                }
                StdOut.println("New Board:");
                board.print();
                break;
        }
    }
    /**
     * Enables students to play the full game of 2048.
     * Calls all methods.
     */
    private static void playFullGame() {
        StdRandom.setSeed(2022);
        Board board = new Board();
        board.updateOpenSpaces();
        board.addRandomTile();
        board.updateOpenSpaces();
        board.addRandomTile();
        StdOut.println("Base Board:");
        board.print();
        StdOut.println("Actions: w = up, a = left, s = down, d = right, q = exit (hit ENTER to enter)");
        HashMap<Character, String> keyMap = new HashMap<>() {
            {
                put('w', "U");
                put('a', "L");
                put('s', "D");
                put('d', "R");
                put('W', "U");
                put('A', "L");
                put('S', "D");
                put('D', "R");
                put('q', "Exit");
            }
        };

        while (true) {
            String toRead = StdIn.readLine();
            String action = keyMap.getOrDefault(toRead.charAt(0), "Invalid");
            if (action.equals("Exit"))
                break;
            if (!action.equals("Invalid")) {
                int[][] oldBoard = new int[4][4];
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        oldBoard[i][j] = board.getBoard()[i][j];
                    }
                }
                board.makeMove(action.charAt(0));
                board.updateOpenSpaces();
                if (!board.isGameLost() && !boardsMatch(oldBoard, board.getBoard())) {
                    board.addRandomTile();
                }
                StdOut.println("New Board: " + action);
                board.print();
            }
        }
    }

    private static boolean boardsMatch(int[][] board1, int[][] board2) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (board1[i][j] != board2[i][j])
                    return false;
            }
        }
        return true;
    }
}