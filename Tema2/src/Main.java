import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.AbstractMap.SimpleEntry;
import java.util.ArrayList;
import java.util.TreeMap;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
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
	public static TreeMap<String,TreeMap<String,Long>> docsFragments = 
			new TreeMap<String, TreeMap<String,Long>>();
	
	public static TreeMap<String, Long> wordCount = new TreeMap<String, Long>();

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
		ArrayList<Future<SimpleEntry<String, TreeMap<String, Long>>>> futures =
				new ArrayList<Future<SimpleEntry<String,TreeMap<String,Long>>>>();
		
		// Assign workers for Map operation
		try {
			for (String document : indexedDocs) {
				// Open file
				RandomAccessFile file = null;
				try {
					file = new RandomAccessFile(document, "r");
				} catch (FileNotFoundException e1) {
					e1.printStackTrace();
				}
				// Assign a worker for each fragment
				for (int i = 0; i <= file.length() / fragmentSize; ++i) {
					Callable<SimpleEntry<String, TreeMap<String, Long>>> worker = 
							new MapWorker(document , i * fragmentSize);
					Future<SimpleEntry<String, TreeMap<String, Long>>> submit = threadPool.submit(worker);
					
					futures.add(submit);
				}
				file.close();
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		// Get Futures (results)
		for (Future<SimpleEntry<String, TreeMap<String, Long>>>future : futures) {
			System.err.println("Future no: ");
			try {
				// TODO: Add Reduce for this work
				/*if (!docsFragments.containsKey(future.get().getKey()))
					docsFragments.put(future.get().getKey(), future.get().getValue());
				else {
					TreeMap<String, Long> tm = docsFragments.get(future.get().getKey());
					//tm.putAll(future.get().getValue());
					for (String w : future.get().getValue().keySet()) {
						if (!tm.containsKey(w))
							tm.put(w, future.get().getValue().get(w));
						else {
							Long oldVal = tm.get(w);
							tm.put(w, oldVal + future.get().getValue().get(w));
						}
					}
					docsFragments.put(future.get().getKey(), tm);
				}*/
				System.err.println("=============" + future.get().getKey() + "\n" + future.get().getValue().toString());
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (ExecutionException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
		}
		
		// Number of words in each document
		for (String doc : wordCount.keySet()) {
			System.err.println("Nr cuvinte document \"" + doc + "\": " + wordCount.get(doc));
		}

		// Terminate all workers
		threadPool.shutdown();
		System.err.println("Terminating thread pool");
		try {
			while (!threadPool.awaitTermination(1, TimeUnit.SECONDS)) {
				System.err.println("Still terminating...");
			}
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		System.err.println("Terminated all threads");
		
		// Create thread Pool for Reduce
		ExecutorService threadPoolReduce = Executors.newFixedThreadPool(NThreads);
		// Get Futures (results)
		for (Future<SimpleEntry<String, TreeMap<String, Long>>>future : futures) {
			System.err.println("Future no: ");
			
			try {
				Runnable r = new ReduceSortWorker(future.get().getKey(), future.get().getValue());
				
				threadPoolReduce.execute(r);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (ExecutionException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			
		}
		
		// Terminate all Reduce workers
		threadPoolReduce.shutdown();
		System.err.println("Terminating thread pool");
		try {
			while (!threadPoolReduce.awaitTermination(1, TimeUnit.SECONDS)) {
				System.err.println("Still terminating...");
			}
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		System.err.println("Terminated all threads");
		
		System.err.println("\n!!!!!FullMap:\n" + docsFragments.toString());
		
	}

	/**
	 * Check and save arguments
	 * 
	 * @param args
	 *            - Program command line arguments
	 */
	private static void checkArgs(String[] args) {
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
	private static void readInput(String inputFileName) {

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
	 * Lowers the characters in strings
	 * 
	 * @param arr
	 *            - The array of Strings to be modified
	 */
	public static void lowerStringArray(ArrayList<String> arr) {
		for (int i = 0; i < arr.size(); ++i) {
			arr.set(i, arr.get(i).toLowerCase());
		}
	}
}
