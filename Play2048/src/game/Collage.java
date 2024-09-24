package game;

import java.awt.Color;
import java.awt.Font;

import java.util.*;
import java.awt.*;
import java.io.File;
import game.StdDraw;
import game.StdOut;
import game.StdIn;
import game.StdRandom;

/**
 * Uses libraries and content from the Introduction to Computer Science book.   
 * Displays board interactively through rendering images with StdDraw.
 * @author Kal Pandit
 **/
public class Collage {
    private int n;

    /**
     * Board collage constructor with default values.
     */
    public Collage() {
        n = 4;
        StdDraw.setCanvasSize(600, 600);
        StdDraw.setScale(0, 4);
        StdDraw.enableDoubleBuffering();
    }

    /**
     * Board collage constructor with an n x n size.
     * 
     * @param size the number of rows and columns in the board
     */
    public Collage(int size) {
        n = size;
        StdDraw.setScale(0, n);
        StdDraw.enableDoubleBuffering();

    }

    /**
     * Updates and refreshes the game board.
     * 
     * @param gb   the game board to reference
     * @param mode whether losses are tracked
     */
    public void updateBoard(Board gb, int mode) {
        int[][] board = gb.getBoard();
        StdDraw.clear();
        if (mode == 2 && gb.isGameLost()) {
            Font font = new Font("Arial", Font.BOLD, 64);
            StdDraw.setPenColor(Color.BLACK);
            String text = "Final score: " + gb.showScore();
            StdDraw.setFont(font);
            StdDraw.text((double) n / 2, (double) n / 2, text);
        } else {
            for (int i = 0; i < board.length; i++) {
                for (int j = 0; j < board[0].length; j++) {
                    replaceTile(i, j, board[i][j]);
                }
            }
        }
        StdDraw.show();
    }

    /**
     * Replaces and draws tiles that need replacing.
     * 
     * @param collageCol   the column on collage's axis
     * @param collageRow   the row on collage's axis
     * @param numToReplace the value of the tile
     */
    public void replaceTile(int collageCol, int collageRow, int numToReplace) {
        String text = Integer.toString(numToReplace);
        if (numToReplace != 0) {
            StdDraw.setPenRadius();
            double centerX = collageRow + .5;
            double centerY = ((n - 1) - collageCol) + .5;
            StdDraw.setPenColor(Color.BLACK);
            StdDraw.square(centerX, centerY, 0.5);
            if (numToReplace == -1) {
                StdDraw.setPenColor(Color.DARK_GRAY);
                StdDraw.filledSquare(centerX, centerY, 0.5);
                StdDraw.setPenColor(Color.WHITE);
                text = "-";
            }
            else if (numToReplace == 2) {
                StdDraw.setPenColor(new Color(238, 228, 218));
                StdDraw.filledSquare(centerX, centerY, 0.5);
                StdDraw.setPenColor(Color.BLACK);
            }
            else if (numToReplace == 4) {
                StdDraw.setPenColor(new Color(255, 224, 200));
                StdDraw.filledSquare(centerX, centerY, 0.5);
                StdDraw.setPenColor(Color.BLACK);
            }
            else if (numToReplace == 8) {
                StdDraw.setPenColor(new Color(242, 177, 121));
                StdDraw.filledSquare(centerX, centerY, 0.5);
                StdDraw.setPenColor(Color.WHITE);
            }
            else if (numToReplace == 16) {
                StdDraw.setPenColor(new Color(245, 149, 99));
                StdDraw.filledSquare(centerX, centerY, 0.5);
                StdDraw.setPenColor(Color.WHITE);           
             }
             else if (numToReplace == 32) {
                StdDraw.setPenColor(new Color(255, 160, 96));
                StdDraw.filledSquare(centerX, centerY, 0.5);
                StdDraw.setPenColor(Color.WHITE);           
             }
             else if (numToReplace == 64) {
                StdDraw.setPenColor(new Color(246, 124, 96));
                StdDraw.filledSquare(centerX, centerY, 0.5);
                StdDraw.setPenColor(Color.WHITE);           
             }
            else {
                StdDraw.setPenColor(new Color(237, 197, 63));
                StdDraw.filledSquare(centerX, centerY, 0.5);
                StdDraw.setPenColor(Color.WHITE);      
            }
            Font font = new Font("Arial", Font.BOLD, 40);
            StdDraw.setFont(font);
            StdDraw.text(centerX, centerY, text);
        } else {
            double centerX = collageRow + .5;
            double centerY = ((n - 1) - collageCol) + .5;
            StdDraw.setPenColor(Color.BLACK);
            StdDraw.setPenRadius(0.001);
            StdDraw.square(centerX, centerY, 0.5);
        }

    }
}
