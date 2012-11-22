import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.AbstractMap.SimpleEntry;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.TreeMap;
import java.util.Vector;
import java.util.concurrent.Callable;
import java.util.concurrent.ConcurrentLinkedQueue;
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
			new TreeMap<String, TreeMap<String,Long>>();	// Document fragments obtained from Map
	
	public static HashMap<String, Long> documentIndices =
			new HashMap<String, Long>();	// Document indices in array
	
	//public static ArrayList<ConcurrentLinkedQueue<TreeMap<String, Long>>> queues =
	//		new ArrayList<ConcurrentLinkedQueue<TreeMap<String,Long>>>();	// Array of queues for reduce
	public static Vector<ConcurrentLinkedQueue<TreeMap<String, Long>>> queues =
			new Vector<ConcurrentLinkedQueue<TreeMap<String,Long>>>();	// Array of queues for reduce
	
	public static ArrayList<ArrayList<Map.Entry<String, Long>>> sortedValues =
			new ArrayList<ArrayList<Map.Entry<String, Long>>>();	// Sorted values for words in files
	
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

		System.err.println("Terminated all Map threads");
		// Number of words in each document
		for (String doc : wordCount.keySet()) {
			System.err.println("Nr cuvinte document \"" + doc + "\": " + wordCount.get(doc));
		}
		
		// Map the filenames with indices
		for (int i = 0; i < indexedDocsNum; ++i)
			documentIndices.put(indexedDocs.get(i), (long) i);
		
		for (int i = 0; i < indexedDocsNum; ++i)
			queues.add(new ConcurrentLinkedQueue<TreeMap<String,Long>>());
		
		
		// Get Futures (results)
		for (Future<SimpleEntry<String, TreeMap<String, Long>>>future : futures) {
			//System.err.println("Future no: ");
			
			/*try {
				Runnable r = new ReduceSortWorker(future.get().getKey(), future.get().getValue());
				
				threadPoolReduce.execute(r);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (ExecutionException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}*/
			
			try {
				String docName = future.get().getKey();
				
				TreeMap<String, Long> chunk = future.get().getValue();
				int index = documentIndices.get(docName).intValue();
				ConcurrentLinkedQueue<TreeMap<String, Long>> tmpqueue = queues.get(index);
				tmpqueue.add(chunk);
				
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (ExecutionException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			
		}
		
		System.err.println("\n\nQueues\n");
		for (int i = 0; i < indexedDocsNum; ++i)
			System.err.println("queue[" + i + "] for doc " + indexedDocs.get(i) + " : " + queues.get(i));
		
		// Create thread Pool for Reduce
		ExecutorService threadPoolReduce = Executors.newFixedThreadPool(NThreads);
		
		
		// TODO reduce code here
		
		boolean enterLoop = true;
		while (enterLoop) {
			enterLoop = false;
			for (int i = 0; i < indexedDocsNum; ++i) {
				ConcurrentLinkedQueue<TreeMap<String, Long>> tmpqueue = queues.get(i);
				if (tmpqueue.size() >= 2) {
					enterLoop = true;
					TreeMap<String, Long> chunk1, chunk2;
					chunk1 = tmpqueue.poll();
					chunk2 = tmpqueue.poll();
					
					//System.err.println("Queues\n" + queues);
					/*String prost = "Prostule";
					prost += queues.toString();
					int lenfsdfd = prost.length();*/
					
					Runnable r = new ReduceAddWordsWorker(indexedDocs.get(i), chunk1, chunk2);
					
					threadPoolReduce.execute(r);
				}
			}
		}
		
		// Terminate all ReduceAddWords workers
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
		System.err.println("Terminated all ReduceAddWords threads");
		
		System.err.println("\n\nQueues\n");
		for (int i = 0; i < indexedDocsNum; ++i)
			System.err.println("queue[" + i + "] for doc " + indexedDocs.get(i) + " : " + queues.get(i));
		
		
		// Initialize sortedValues
		for (int i = 0; i < indexedDocsNum; ++i) {
			sortedValues.add(new ArrayList<Map.Entry<String,Long>>());
		}
		
		// Create thread pool for ReduceSortWorkers
		threadPoolReduce = Executors.newFixedThreadPool(NThreads);
		for (int i = 0; i < indexedDocsNum; ++i) {
			Runnable r = new ReduceReverseSortWorker(i, queues.get(i).element());
			
			threadPoolReduce.execute(r);
		}
		
		// Terminate all Reduce Sort workers
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
		
		// Debug print sorted arrays
		for (int i = 0; i < indexedDocsNum; ++i) {
			System.err.println("sorted for file " + indexedDocs.get(i) + " : " + sortedValues.get(i));
		}
		
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
