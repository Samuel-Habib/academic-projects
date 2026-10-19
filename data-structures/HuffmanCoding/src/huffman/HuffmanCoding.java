package huffman;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.Collections;

/**
 * This class contains methods which, when used together, perform the
 * entire Huffman Coding encoding and decoding process
 * 
 * @author Ishaan Ivaturi
 * @author Prince Rawal
 */
public class HuffmanCoding {
    private String fileName;
    private ArrayList<CharFreq> sortedCharFreqList;
    private TreeNode huffmanRoot;
    private String[] encodings;

    /**
     * Constructor used by the driver, sets filename
     * DO NOT EDIT
     * @param f The file we want to encode
     */
    public HuffmanCoding(String f) { 
        fileName = f; 
    }

    /**
     * Reads from filename character by character, and sets sortedCharFreqList
     * to a new ArrayList of CharFreq objects with frequency > 0, sorted by frequency
     */
    public void makeSortedList() {
        StdIn.setFile(fileName);

    /* Your code goes here */

        // charFreq(char, double)

        sortedCharFreqList = new ArrayList<CharFreq>();
        int[] a = new int[128];
        int counter = 0;
        while(StdIn.hasNextChar()){
            char c = StdIn.readChar();
            a[c]++;
            counter++;
        // the counter now has the total number of characters in the file
        }

        for(int i = 0; i < a.length; i++){
            if(a[i] != 0){
                CharFreq newCF = new CharFreq((char) i, (double) a[i]/counter);
                sortedCharFreqList.add(newCF);
            }
        }
        if(sortedCharFreqList.size() == 1){
            CharFreq remainCF = new CharFreq((char)(sortedCharFreqList.get(0).getCharacter()+ 1), 0.0);
            sortedCharFreqList.add(remainCF);
        }

        Collections.sort(sortedCharFreqList);

    }

    /**
     * Uses sortedCharFreqList to build a huffman coding tree, and stores its root
     * in huffmanRoot
     * 
     */

    public void makeTree() {
    
    Queue<TreeNode> source = new Queue<TreeNode>();
    Queue<TreeNode> target = new Queue<TreeNode>();

    for(int i = 0; i < sortedCharFreqList.size(); i++){
        source.enqueue(new TreeNode(sortedCharFreqList.get(i), null, null));
    }

    while(!source.isEmpty() || target.size() != 1 ) {
        TreeNode n = new TreeNode();
        TreeNode left = null;
        TreeNode right = null;
        CharFreq combined = new CharFreq();
        combined.setCharacter((char) 0);
        

        if(target.size() == 0){
            if(source.size() != 1){
                left = source.dequeue();
                right = source.dequeue();
            }else{
                combined.setProbOcc(1.0);
                left = source.dequeue();
            }
        }else{
            if(!source.isEmpty()){
                    double SourceProbOcc = source.peek().getData().getProbOcc();
                    double targetProbOcc = target.peek().getData().getProbOcc();
                    if(SourceProbOcc <= targetProbOcc){
                        left = source.dequeue();
                    }
                    else{
                        left = target.dequeue();
                    }
                    if(source.isEmpty()){
                        right = target.dequeue();
                    }else{
                        SourceProbOcc = source.peek().getData().getProbOcc();
                        if(!target.isEmpty()){
                            targetProbOcc = target.peek().getData().getProbOcc();
                            if(SourceProbOcc <= targetProbOcc){
                                right = source.dequeue();
                            }
                            else{
                                right = target.dequeue();
                            }
                        }else{
                            right = source.dequeue();
                            
                        }
                    }              
            }else{
                left = target.dequeue();
                if(!target.isEmpty()){
                    right = target.dequeue();
                }
            }
        }

        n.setLeft(left);
        if(right != null){
            n.setRight(right);
            double SourceProbOcc = left.getData().getProbOcc() + right.getData().getProbOcc();
            combined.setProbOcc(SourceProbOcc);
        }
        n.setData(combined);
        target.enqueue(n);
    }
    huffmanRoot = target.peek();
    }
        
    /**
     * Uses huffmanRoot to create a string array of size 128, where each
     * index in the array contains that ASCII character's bitstring encoding. Characters not
     * present in the huffman coding tree should have their spots in the array left null.
     * Set encodings to this array.
     */
    public void makeEncodings() {
        /* Your code goes here */

        encodings = new String[128];
        TreeNode n = huffmanRoot;

        String encode = "";
        helper(n, encode, encodings);
    
    }
    public static void helper(TreeNode nn, String e, String[] ee){
        if(nn == null){
            return;
        }
        if(nn.getData().getCharacter() != (char) 0){
            ee[nn.getData().getCharacter()] = e;
        }
        helper(nn.getLeft(), e + "0", ee);
        helper(nn.getRight(), e + "1", ee);
    }

    /**
     * Using encodings and filename, this method makes use of the writeBitString method
     * to write the final encoding of 1's and 0's to the encoded file.
     * 
     * @param encodedFile The file name into which the text file is to be encoded
     */
    public void encode(String encodedFile) {
        StdIn.setFile(fileName);
        /* Your code goes here */
        String encode = "";
        while(StdIn.hasNextChar()){
            char c = StdIn.readChar();
            if(encodings[(int)c] != null){
                encode += encodings[(int)c];
            }
        }
        writeBitString(encodedFile,encode);
    }
    
    /**
     * Writes a given string of 1's and 0's to the given file byte by byte
     * and NOT as characters of 1 and 0 which take up 8 bits each
     * DO NOT EDIT
     * 
     * @param filename The file to write to (doesn't need to exist yet)
     * @param bitString The string of 1's and 0's to write to the file in bits
     */
    public static void writeBitString(String filename, String bitString) {
        byte[] bytes = new byte[bitString.length() / 8 + 1];
        int bytesIndex = 0, byteIndex = 0, currentByte = 0;

        // Pad the string with initial zeroes and then a one in order to bring
        // its length to a multiple of 8. When reading, the 1 signifies the
        // end of padding.
        int padding = 8 - (bitString.length() % 8);
        String pad = "";
        for (int i = 0; i < padding-1; i++) pad = pad + "0";
        pad = pad + "1";
        bitString = pad + bitString;

        // For every bit, add it to the right spot in the corresponding byte,
        // and store bytes in the array when finished
        for (char c : bitString.toCharArray()) {
            if (c != '1' && c != '0') {
                System.out.println("Invalid characters in bitstring");
                return;
            }

            if (c == '1') currentByte += 1 << (7-byteIndex);
            byteIndex++;
            
            if (byteIndex == 8) {
                bytes[bytesIndex] = (byte) currentByte;
                bytesIndex++;
                currentByte = 0;
                byteIndex = 0;
            }
        }
        
        // Write the array of bytes to the provided file
        try {
            FileOutputStream out = new FileOutputStream(filename);
            out.write(bytes);
            out.close();
        }
        catch(Exception e) {
            System.err.println("Error when writing to file!");
        }
    }

    /**
     * Using a given encoded file name, this method makes use of the readBitString method 
     * to convert the file into a bit string, then decodes the bit string using the 
     * tree, and writes it to a decoded file. 
     * 
     * @param encodedFile The file which has already been encoded by encode()
     * @param decodedFile The name of the new file we want to decode into
     */
    public void decode(String encodedFile, String decodedFile) {
        StdOut.setFile(decodedFile);
        /* Your code goes here */
        String encodedData = readBitString(encodedFile);
        char charBit;
        String finalstring = "";
        int i = 0;
        TreeNode n = huffmanRoot;
        while(i < encodedData.length()) {
            charBit = encodedData.charAt(i);
            if(Character.toString(charBit).equals("1")){
                n = n.getRight();
                if(n.getData().getCharacter() != (char) 0){
                    finalstring += n.getData().getCharacter();
                    n = huffmanRoot;
                }
            }else{
                n = n.getLeft();
                if(n.getData().getCharacter() != (char) 0){
                    finalstring += n.getData().getCharacter();
                    n = huffmanRoot;
                }
            }
            i++;
        }
        StdOut.print(finalstring);
        /* Your code goes here */
    }

    /**
     * Reads a given file byte by byte, and returns a string of 1's and 0's
     * representing the bits in the file
     * DO NOT EDIT
     * 
     * @param filename The encoded file to read from
     * @return String of 1's and 0's representing the bits in the file
     */
    public static String readBitString(String filename) {
        String bitString = "";
        
        try {
            FileInputStream in = new FileInputStream(filename);
            File file = new File(filename);

            byte bytes[] = new byte[(int) file.length()];
            in.read(bytes);
            in.close();
            
            // For each byte read, convert it to a binary string of length 8 and add it
            // to the bit string
            for (byte b : bytes) {
                bitString = bitString + 
                String.format("%8s", Integer.toBinaryString(b & 0xFF)).replace(' ', '0');
            }

            // Detect the first 1 signifying the end of padding, then remove the first few
            // characters, including the 1
            for (int i = 0; i < 8; i++) {
                if (bitString.charAt(i) == '1') return bitString.substring(i+1);
            }
            
            return bitString.substring(8);
        }
        catch(Exception e) {
            System.out.println("Error while reading file!");
            return "";
        }
    }

    /*
     * Getters used by the driver. 
     * DO NOT EDIT or REMOVE
     */

    public String getFileName() { 
        return fileName; 
    }

    public ArrayList<CharFreq> getSortedCharFreqList() { 
        return sortedCharFreqList; 
    }

    public TreeNode getHuffmanRoot() { 
        return huffmanRoot; 
    }

    public String[] getEncodings() { 
        return encodings; 
    }
}
