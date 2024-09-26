package kindergarten;

/**
 * This class represents a student node, with a student 
 * object representing the student, and a next pointer 
 * for the student next in line.
 * 
 * @author Ethan Chou
 */
public class SNode {
    private Student student; // the data part of the node
    private SNode   next;    // a link to the next student int the linked list

    public SNode ( Student s, SNode n ) {
		student = s;
		next = n;
	}

    public SNode() {
        this(null, null);
    }

    public Student getStudent () { return student; }
    public void setStudent (Student s) { student = s; }

    public SNode getNext () { return next; }
    public void setNext (SNode n) { next = n; }
}
