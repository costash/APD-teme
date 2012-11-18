import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

/**
 * @author Constantin Șerban-Rădoi 333CA
 * @category Main class containing main() function
 */
public class Main {

	public static int nrCuvCheie; // Numarul de cuvinte cheie de cautat
	public static ArrayList<String> cuvCheie = new ArrayList<String>(); // Cuvintele
																		// cheie
																		// de
																		// cautat
	public static int fragmentSize; // Dimensiunea in Octeti a unui fragment in
									// care este impartit un fisier
	public static int mostFrequentN; // Numarul de cuvinte cele mai frecvente
										// retinute pentru fiecare document
	public static int numDocumentsX; // Numarul de documente pentru care vreau
										// sa primesc rezultat (cele mai
										// relevante
										// X documente
	public static int indexedDocsNum; // Numarul de documente de indexat
	public static ArrayList<String> indexedDocs = new ArrayList<String>(); // Numele
																			// documentelor
																			// de
																			// indexat

	public static String inputFileName; // Input file name
	public static String outputFileName; // Output file name
	public static int NThreads; // Number of threads

	/**
	 * @param args
	 *            - Program command line arguments
	 */
	public static void main(String[] args) {

		checkArgs(args);

		System.err.println("NT:" + args[0] + " in: " + args[1] + " out: "
				+ args[2] + "\n");

		readInput(inputFileName);

		System.err.println("NrCuvCheie: " + nrCuvCheie);
		System.err.println("CuvCheie:\n" + cuvCheie.toString());
		System.err.println("FragmentSize:\t" + fragmentSize);
		System.err.println("MostFrequentN:\t" + mostFrequentN);
		System.err.println("NumberOfMostRelevantXDocs:\t" + numDocumentsX);
		System.err.println("Number of indexed docs:\t" + indexedDocsNum);
		System.err.println("Indexed Docs:\n" + indexedDocs.toString());

		// Create a thread pool
		ExecutorService threadPool = Executors.newFixedThreadPool(NThreads);
		for (int i = 0; i < NThreads; ++i) {
			Runnable worker = new MapWorker();
			threadPool.execute(worker);
			System.err.println(i);
		}

		// Terminate all workers
		threadPool.shutdown();
		System.err.println("Terminating tread pool");
		try {
			while (!threadPool.awaitTermination(1, TimeUnit.SECONDS)) {
				System.err.println("Still terminating...");
			}
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		System.err.println("Terminated all threads");

		// Test code:
		ArrayList<String> chunkWords = new ArrayList<String>();
		/*
		 * boolean prevWordEnded = getNextChunk(indexedDocs.get(0), 4000,
		 * chunkWords, false); System.err.println("\n\"" + chunkWords +
		 * "\"\nPrevWordEnded: " + prevWordEnded);
		 */
		// Open file
		RandomAccessFile file = null;
		try {
			file = new RandomAccessFile(indexedDocs.get(0), "r");
		} catch (FileNotFoundException e1) {
			e1.printStackTrace();
		}
		try {
			boolean prevWordEnded = false;
			for (int i = 0; i <= file.length() / fragmentSize; ++i) {
				prevWordEnded = getNextChunk(indexedDocs.get(0), i
						* fragmentSize, chunkWords, prevWordEnded);

				System.err.println("\n" + chunkWords);
				chunkWords.clear();
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	/**
	 * Check and save arguments
	 * 
	 * @param args
	 *            - Program command line arguments
	 */
	static void checkArgs(String[] args) {
		if (args.length != 3) {
			System.err
					.println("Error! Must have exactly three parameters. Usage:\n\tNThreads input_file output_file\n");
			System.exit(1);
		}
		NThreads = Integer.parseInt(args[0]);
		inputFileName = args[1];
		outputFileName = args[2];
	}

	/**
	 * Read input from file
	 * 
	 * @param inputFileName
	 *            - Filename for reading input
	 */
	static void readInput(String inputFileName) {

		try {
			BufferedReader input = new BufferedReader(new FileReader(
					inputFileName));
			try {
				String line = null;

				if ((line = input.readLine()) != null) {
					nrCuvCheie = Integer.parseInt(line);
				}
				// System.err.println("NrCuvCheie: " + nrCuvCheie);

				if ((line = input.readLine()) != null) {
					String[] words = line.split(" ");
					for (String word : words) {
						cuvCheie.add(word);
					}
				}
				// System.err.println("CuvCheie:\n" + cuvCheie.toString());

				if ((line = input.readLine()) != null) {
					fragmentSize = Integer.parseInt(line);
				}
				// System.err.println("FragmentSize:\t" + fragmentSize);

				if ((line = input.readLine()) != null) {
					mostFrequentN = Integer.parseInt(line);
				}
				// System.err.println("MostFrequentN:\t" + mostFrequentN);

				if ((line = input.readLine()) != null) {
					numDocumentsX = Integer.parseInt(line);
				}
				// System.err.println("NumberOfMostRelevantXDocs:\t" +
				// numDocumentsX);

				if ((line = input.readLine()) != null) {
					indexedDocsNum = Integer.parseInt(line);
				}
				// System.err.println("Number of indexed docs:\t" +
				// indexedDocsNum);

				for (int i = 0; i < indexedDocsNum; ++i) {
					if ((line = input.readLine()) != null) {
						indexedDocs.add(new String(line));
					}
				}
				// System.err.println("Indexed Docs:\n" +
				// indexedDocs.toString());

			} finally {
				input.close();
			}
		} catch (IOException ex) {
			ex.printStackTrace();
		}
	}

	/**
	 * Gets a new array of words by reading a new chunk from file
	 * 
	 * @param fileName
	 *            - The filename to read from
	 * @param pos
	 *            - The position in file where to start from
	 * @param chunkWords
	 *            - The array of words. Must not be null
	 * @param previewsWordEnded
	 *            - Whether the previews chunk ended with a space or not
	 * @return True if the chunk's last word ended, False otherwise
	 */
	static boolean getNextChunk(String fileName, long pos,
			ArrayList<String> chunkWords, boolean previewsWordEnded) {
		boolean lastWordEnded = false;

		// Open file
		RandomAccessFile file = null;
		try {
			file = new RandomAccessFile(fileName, "r");
		} catch (FileNotFoundException e1) {
			e1.printStackTrace();
		}

		try {
			// Seek to the next chunk position
			file.seek(pos);

			// Read fragmentSize bytes from current position in file
			int chunkSize = fragmentSize;
			long fLength = file.length();
			System.err.println("File len " + fLength);
			if (pos + chunkSize > fLength)
				chunkSize = (int) (fLength - pos);

			System.err.println("Chunk size: " + chunkSize);

			// Try to read chunkSize bytes;
			byte[] b = new byte[chunkSize];
			file.read(b, 0, chunkSize);

			String chunk = new String(b);
			System.err.print("Chunk:\t>>" + chunk + "<<");

			int first = 0;
			boolean skipped = false;
			// Skip the first word
			while (!previewsWordEnded && first < chunkSize - 1
					&& Character.isLetter(chunk.charAt(first))) {
				++first;
				skipped = true;
			}
			if (skipped)
				++first; // I am at first non-white-space now

			// Skip the last word
			int last = (int) (chunkSize - 1);
			while (last > 0 && Character.isLetter(chunk.charAt(last))) {
				--last;
			}

			System.err.println("\nFirst non-space: " + first
					+ " last non-space: " + last);
			if (first < chunk.length())
				System.err.println("chunk[" + first + "]= {"
						+ chunk.charAt(first) + "} chunk[" + last + "]= {"
						+ chunk.charAt(last) + "}");

			StringBuilder sbw = new StringBuilder();
			// Chunk ended with anything else than a letter, so last word is
			// completed
			if (last == chunkSize - 1)
				lastWordEnded = true;
			else {
				int idx = 0;
				while (idx + last < chunkSize - 1) {
					idx++;
					sbw.append(chunk.charAt(idx + last));
				}

				// Complete the last word
				byte bt = 0;
				while (idx + last + pos < fLength
						&& Character.isLetter((char) (bt = file.readByte()))) {
					sbw.append((char) (bt));
					idx++;
				}
				System.err.println("Last word: {" + sbw.toString() + "}");

				lastWordEnded = false;
			}

			tokenizeString(chunkWords, chunk, first, last);

			if (lastWordEnded == false)
				chunkWords.add(sbw.toString());

		} catch (IOException e) {
			e.printStackTrace();
		}

		return lastWordEnded;
	}

	/**
	 * Tokenizes a String into words
	 * 
	 * @param chunkWords
	 *            - The Array where the words are saved
	 * @param chunk
	 *            - The chunk to be tokenized
	 * @param first
	 *            - Position in chunk for start
	 * @param last
	 *            - Position in chunk for end
	 */
	private static void tokenizeString(ArrayList<String> chunkWords,
			String chunk, int first, int last) {
		StringBuilder sb = new StringBuilder();
		while (first <= last) {
			char c = chunk.charAt(first);

			int len;
			if (Character.isLetter(c)) {
				sb.append(c);
			} else if ((len = sb.length()) > 0) {
				chunkWords.add(sb.toString());
				sb.delete(0, len);
			}

			++first;
		}
	}

	/**
	 * Lowers the characters in strings
	 * 
	 * @param arr
	 *            - The array of Strings to be modified
	 */
	static void lowerStringArray(ArrayList<String> arr) {
		for (int i = 0; i < arr.size(); ++i) {
			arr.set(i, arr.get(i).toLowerCase());
		}
	}
}
