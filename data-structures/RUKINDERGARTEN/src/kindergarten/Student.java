package kindergarten;

/**
 * This class represents a student, with a string variable for the 
 * student's name and an int variable for the student's height.
 * 
 * @author Ethan Chou
 */
public class Student {
    private String firstName;
    private String lastName;
    private int    height;

    /*
     * Constructor  
     */
    public Student ( String f, String l, int h ) {
		firstName = f;
        lastName  = l;
		height    = h;
	}

    /*
     * Default constructor sets student's height to 0 (zero)
     */ 
    public Student () {
        this(null, null, 0);
    }

    /**
     * Compares the names of this student to paremeter student, 
     * returning an int value representing which name comes in first alphabetically.
     * 
     * @param other the student being compared
     * 
     * @return an int value of which name comes first alphabetically
     * n < 0 means this student's name comes before parameter student
     * n > 0 means parameter student's name comes before this student
     */
    public int compareNameTo ( Student other ) {
        
        int lastNameCompare = this.lastName.compareToIgnoreCase(other.getLastName());
        
        if ( lastNameCompare == 0 ) {
            // same last name, compare first names
            return this.firstName.compareToIgnoreCase(other.getFirstName());
        }
        return lastNameCompare;
    }

    /*
     * Prints the student object
     */
    public String print () {
        String printName = "";
        printName += this.firstName.charAt(0) + ". ";
        if ( lastName.length() > 4 ) {
            printName += this.lastName.substring(0, 4);
        } else {
            printName += lastName;
        }
        return printName;
    }

    /* Getter and setter methods */
    public String getFirstName () { return firstName; }
    public void setFirstName ( String f ) { firstName = f; }

    public String getLastName () { return lastName; }
    public void setLastName ( String l ) { lastName = l; }

    public int getHeight () { return height; }
    public void setHeight ( int h ) { height = h; }
    
    public String getFullName () { return this.firstName + " " + this.lastName;}
}
