import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

/**
 * 
 */

/**
 * @author Constantin Șerban-Rădoi 333CA
 *
 */
public class Main {

	public static int nrCuvCheie;		// Numarul de cuvinte cheie de cautat
	public static ArrayList<String> cuvCheie = new ArrayList<String>();	// Cuvintele cheie de cautat
	public static int fragmentSize;		// Dimensiunea in Octeti a unui fragment in care este impartit un fisier
	public static int mostFrequentN;	// Numarul de cuvinte cele mai frecvente retinute pentru fiecare document
	public static int numDocumentsX;	// Numarul de documente pentru care vreau sa primesc rezultat (cele mai relevante
										// X documente
	public static int indexedDocsNum;		// Numarul de documente de indexat
	public static ArrayList<String> indexedDocs = new ArrayList<String>();	// Numele documentelor de indexat
	
	public static String inputFileName;		// Input file name
	public static String outputFileName;	// Output file name
	public static int NThreads;				// Number of threads
	/**
	 * @param args Program command line arguments
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		
		checkArgs(args);
		
		System.err.println("NT:" + args[0] + " in: " + args[1] + " out: " + args[2] + "\n");
		
		readInput(args[1]);
		
		System.err.println("NrCuvCheie: " + nrCuvCheie);
		System.err.println("CuvCheie:\n" + cuvCheie.toString());
		System.err.println("FragmentSize:\t" + fragmentSize);
		System.err.println("MostFrequentN:\t" + mostFrequentN);
		System.err.println("NumberOfMostRelevantXDocs:\t" + numDocumentsX);
		System.err.println("Number of indexed docs:\t" + indexedDocsNum);
		System.err.println("Indexed Docs:\n" + indexedDocs.toString());
	}
	
	/**
	 * Check and save arguments
	 * @param args Program command line arguments
	 */
	static void checkArgs(String[] args) {
		if (args.length != 3) {
			System.err.println("Error! Must have exactly three parameters. Usage:\n\tNThreads input_file output_file\n");
			System.exit(1);
		}
		NThreads = Integer.parseInt(args[0]);
		inputFileName = args[1];
		outputFileName = args[2];
	}
	
	/**
	 * Read input from file
	 * @param inputFileName Filename for reading input
	 */
	static void readInput(String inputFileName) {
		
		try {
			BufferedReader input = new BufferedReader(new FileReader(inputFileName));
			try {
				String line = null;
				
				if ( (line = input.readLine()) != null) {
					nrCuvCheie = Integer.parseInt(line);
				}
				//System.err.println("NrCuvCheie: " + nrCuvCheie);
				
				if ( (line = input.readLine()) != null) {
					String[] words = line.split(" ");
					for (String word : words) {
						cuvCheie.add(word);
					}
				}
				//System.err.println("CuvCheie:\n" + cuvCheie.toString());
				
				if ( (line = input.readLine()) != null) {
					fragmentSize = Integer.parseInt(line);
				}
				//System.err.println("FragmentSize:\t" + fragmentSize);
				
				if ( (line = input.readLine()) != null) {
					mostFrequentN = Integer.parseInt(line);
				}
				//System.err.println("MostFrequentN:\t" + mostFrequentN);
				
				if ( (line = input.readLine()) != null) {
					numDocumentsX = Integer.parseInt(line);
				}
				//System.err.println("NumberOfMostRelevantXDocs:\t" + numDocumentsX);
				
				if ( (line = input.readLine()) != null) {
					indexedDocsNum = Integer.parseInt(line);
				}
				//System.err.println("Number of indexed docs:\t" + indexedDocsNum);
				
				for (int i = 0; i < indexedDocsNum; ++i) {
					if ( (line = input.readLine()) != null) {
						indexedDocs.add(new String(line));
					}
				}
				//System.err.println("Indexed Docs:\n" + indexedDocs.toString());
				
			}
			finally {
				input.close();
			}
		}
		catch (IOException ex){
			ex.printStackTrace();
		}
	}

}
