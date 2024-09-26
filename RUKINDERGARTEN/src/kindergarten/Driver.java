package kindergarten;
import java.util.*;
import java.io.File;
/**
 * This class is designed to test each method in the Transit file interactively
 * 
 * @author Ethan Chou 
 * @author Maksims Kurjanovics Kravcenko
 * @author Kal Pandit
 */

public class Driver {
    public static void main(String[] args) {
		String[] methods = {"enterClassroom", "setupSeats", "seatStudents", 
		"insertMusicalChairs", "eliminateLosingStudents", "moveStudentFromChairsToLine", "seatMusicalChairsWinner",
		"playMusicalChairs"};
		String[] options = {"Test a new input file", "Test another method on the same file", "Quit"};
		int controlChoice = 1;
        Classroom studentClassroom = new Classroom();
		
		do {
			StdOut.print("Enter a student info input file => ");
			String inputFile = StdIn.readLine();

			do {
				StdOut.println("\nWhat method would you like to test?");
				for (int i = 0; i < methods.length; i++) {
					StdOut.printf("%d. %s\n", i+1, methods[i]);
				}
				StdOut.print("Enter a number => ");
				int choice = Integer.parseInt(StdIn.readLine());
				switch (choice) {
					case 1:
					// enterClassroom
						studentClassroom = testEnterClassroom(inputFile);
						studentClassroom.printClassroom();
					break;
                    case 2:
					// setupSeats
						StdOut.print("Enter a seating availability input file => ");
						String seating = StdIn.readLine();
						testSetupSeats(studentClassroom, seating);
						studentClassroom.printClassroom();
                        break;
					case 3:
					// seatStudents
						testSeatStudents(studentClassroom);
						studentClassroom.printClassroom();
						break;
					case 4:
					// insertMusicalChairs
						testInsertMusicalChairs(studentClassroom);
						studentClassroom.printClassroom();
						break;
					case 5:
						StdRandom.setSeed(2022);
					// eliminateLosingStudents
						studentClassroom.eliminateLosingStudents();
						studentClassroom.printClassroom();
						break;
					case 6:
						StdRandom.setSeed(2022);
					// moveStudentFromMusicalChairsToLine
						StdOut.print("Enter the size of the circular linked list --> ");
						int size = Integer.parseInt(StdIn.readLine());
						studentClassroom.moveStudentFromChairsToLine(size);
						studentClassroom.printClassroom();
						break;
					case 7:
					// seatMusicalChairsWinner
						testSeatMusicalChairsWinner(studentClassroom);
						studentClassroom.printClassroom();
						break;
					case 8:
					// playMusicalChairs -- calls all musical chairs methods in sequence
						StdRandom.setSeed(2022);
						testPlayMusicalChairs(studentClassroom);
						studentClassroom.printClassroom();
						break;
					default:
						StdOut.println("Not a valid option!");
				}
				StdIn.resetFile();
				StdOut.println("What would you like to do now?");
				for (int i = 0; i < 3; i++) {
					StdOut.printf("%d. %s\n", i+1, options[i]);
				}
				StdOut.print("Enter a number => ");
				controlChoice = Integer.parseInt(StdIn.readLine());
			} while (controlChoice == 2);
		} while (controlChoice == 1);
    }

    private static Classroom testEnterClassroom(String filename) {
		// Call student's makeList method
	    Classroom studentClassroom = new Classroom();
		studentClassroom.enterClassroom(filename);
		return studentClassroom;
    }
	private static void testSetupSeats(Classroom studentClassroom, String filename) {
		studentClassroom.setupSeats(filename);
	} 

	private static void testSeatStudents(Classroom studentClassroom) {
		studentClassroom.seatStudents();
	}
	private static void testEliminateLosingStudents(Classroom studentClassroom) {
		studentClassroom.eliminateLosingStudents();
	}
	private static void testInsertMusicalChairs(Classroom studentClassroom) {
		studentClassroom.insertMusicalChairs();
	}
	private static void testSeatMusicalChairsWinner(Classroom studentClassroom) {
		studentClassroom.seatMusicalChairsWinner();
	}
	private static void testInsertByHeight(Classroom studentClassroom) {
		StdOut.print("\nWrite the student's first name -> ");
        String studentName = StdIn.readString();
		StdOut.print("\nWrite the student's last name -> ");
        String lastName = StdIn.readString();
		StdOut.print("\nWrite the student's height as a number -> ");
        int height = StdIn.readInt();
		Student toAdd = new Student(studentName, lastName, height); 
		studentClassroom.insertByHeight(toAdd);
	}
	
	private static void testPlayMusicalChairs(Classroom studentClassroom) {
		studentClassroom.playMusicalChairs();		
	}
}
