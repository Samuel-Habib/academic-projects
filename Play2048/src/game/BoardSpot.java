package game;

import java.util.*;
import java.awt.*;
import java.io.File;
import game.StdDraw;
import game.StdOut;
import game.StdIn;
import game.StdRandom;

/**
 * BoardSpot class -- used to indicate and store data for open board spots.
 * 
 * @author Ishaan Ivaturi
 * 
 */
public class BoardSpot {
    private int row;
    private int col;

    /**
     * Board spot constructor with default values.
     */
    public BoardSpot() {
        row = 0;
        col = 0;
    }

    /**
     * Board spot constructor with row and column values given in parameters.
     * 
     * @param rowVal the given row
     * @param columnVal the given column
     */
    public BoardSpot(int rowVal, int columnVal) {
        row = rowVal;
        col = columnVal;
    }

    /**
     * Obtains the row of a board spot.
     * 
     * @return The row of a board spot.
     */
    public int getRow() {
        return row;
    }

    /**
     * Obtains the column of a board spot.
     * 
     * @return The column of a board spot.
     */
    public int getCol() {
        return col;
    }

    /**
     * Compares this BoardSpot to another
     * 
     * @param other the object to compare against @this
     * @return true if @this and @other have the same (row,col)
     */
    public boolean equals (Object other) {

        if ( other instanceof BoardSpot ) {
            BoardSpot o = (BoardSpot) other;
            return row == o.row && col == o.col;
        } else {
            return false;
        }
    }
}
