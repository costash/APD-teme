import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
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
import java.util.concurrent.atomic.AtomicInteger;

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
	public static TreeMap<String, TreeMap<String, Long>> docsFragments = new TreeMap<String, TreeMap<String, Long>>(); // Document
																														// fragments
																														// obtained
																														// from
																														// Map

	public static HashMap<String, Long> documentIndices = new HashMap<String, Long>(); // Document
																						// indices
																						// in
																						// array

	public static Vector<ConcurrentLinkedQueue<TreeMap<String, Long>>> queues = new Vector<ConcurrentLinkedQueue<TreeMap<String, Long>>>(); // Array
																																			// of
																																			// queues
																																			// for
																																			// reduce

	public static Vector<AtomicInteger> queuesLock = new Vector<AtomicInteger>(); // Lock
																					// for
																					// reduce
																					// loop

	public static Vector<String> searchResults = new Vector<String>(); // Search
																		// results
																		// as
																		// Strings

	public static ArrayList<ArrayList<Map.Entry<String, Long>>> sortedValues = new ArrayList<ArrayList<Map.Entry<String, Long>>>(); // Sorted
																																	// values
																																	// for
																																	// words
																																	// in
																																	// files

	public static TreeMap<String, Long> wordCount = new TreeMap<String, Long>();

	public static String inputFileName; // Input file name
	public static String outputFileName; // Output file name
	public static int NThreads; // Number of threads

	public static final boolean DEBUG = false;

	/**
	 * @param args
	 *            - Program command line arguments
	 */
	public static void main(String[] args) {

		checkArgs(args);
		if (DEBUG) {
			System.err.println("NT:" + args[0] + " in: " + args[1] + " out: "
					+ args[2] + "\n");
		}

		readInput(inputFileName);

		if (DEBUG) {
			System.err.println("NrCuvCheie: " + nrCuvCheie);
			System.err.println("CuvCheie:\n" + cuvCheie.toString());
			System.err.println("FragmentSize:\t" + fragmentSize);
			System.err.println("MostFrequentN:\t" + mostFrequentN);
			System.err.println("NumberOfMostRelevantXDocs:\t" + numDocumentsX);
			System.err.println("Number of indexed docs:\t" + indexedDocsNum);
			System.err.println("Indexed Docs:\n" + indexedDocs.toString());
		}

		ArrayList<Future<SimpleEntry<String, TreeMap<String, Long>>>> futures = mapOperations();

		// Number of words in each document
		if (DEBUG) {
			for (String doc : wordCount.keySet()) {
				System.err.println("Nr cuvinte document \"" + doc + "\": "
						+ wordCount.get(doc));
			}
		}
		initQueues();
		getMapResults(futures);

		if (DEBUG) {
			System.err.println("\n\nQueues\n");
			for (int i = 0; i < indexedDocsNum; ++i)
				System.err.println("queue[" + i + "] for doc "
						+ indexedDocs.get(i) + " : " + queues.get(i));
		}

		reduceFirstTask();

		if (DEBUG) {
			System.err.println("\n\nQueues\n");
			for (int i = 0; i < indexedDocsNum; ++i)
				System.err.println("queue[" + i + "] for doc "
						+ indexedDocs.get(i) + " : " + queues.get(i));
		}

		initResults();
		reduceSecondTask();

		if (DEBUG) {
			// Debug print sorted arrays
			for (int i = 0; i < indexedDocsNum; ++i) {
				System.err.println("sorted for file " + indexedDocs.get(i)
						+ " : " + sortedValues.get(i));
			}
		}

		StringBuilder sb = getFinalOutput();

		// This is the final result to be written to file output
		writeOutput(sb);
	}

	/**
	 * Inits the results array and sortedValues array
	 */
	private static void initResults() {
		// Initialize sortedValues
		for (int i = 0; i < indexedDocsNum; ++i) {
			sortedValues.add(new ArrayList<Map.Entry<String, Long>>());
		}

		// Initialize search results
		for (int i = 0; i < indexedDocsNum; ++i)
			searchResults.add(new String(""));
	}

	/**
	 * Execute the second reduce task which selects the documents
	 */
	private static void reduceSecondTask() {
		// Create thread pool for ReduceSortWorkers
		ExecutorService threadPoolReduce = Executors
				.newFixedThreadPool(NThreads);
		for (int i = 0; i < indexedDocsNum; ++i) {
			Runnable r = new ReduceReverseSortWorker(i, queues.get(i).element());

			threadPoolReduce.execute(r);
		}

		// Terminate all Reduce Sort workers
		threadPoolReduce.shutdown();
		if (DEBUG) {
			System.err.println("Terminating thread pool");
		}
		try {
			while (!threadPoolReduce.awaitTermination(1, TimeUnit.SECONDS)) {
				if (DEBUG)
					System.err.println("Still terminating...");
			}
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	/**
	 * Execute the first reduce task which makes the reunion of map results
	 */
	private static void reduceFirstTask() {
		// Create thread Pool for Reduce
		ExecutorService threadPoolReduce = Executors
				.newFixedThreadPool(NThreads);

		boolean enterLoop = true;
		while (enterLoop) {
			enterLoop = false;
			for (int i = 0; i < indexedDocsNum; ++i) {
				ConcurrentLinkedQueue<TreeMap<String, Long>> tmpqueue = queues
						.get(i);

				AtomicInteger lock = queuesLock.get(i);
				if (lock.get() > 0) {
					enterLoop = true;
				}

				if (tmpqueue.size() >= 2) {
					enterLoop = true;
					TreeMap<String, Long> chunk1, chunk2;
					chunk1 = tmpqueue.poll();
					chunk2 = tmpqueue.poll();

					// Increment lock on queue
					lock.incrementAndGet();
					Runnable r = new ReduceAddWordsWorker(indexedDocs.get(i),
							chunk1, chunk2);

					threadPoolReduce.execute(r);
				}
			}
		}

		// Terminate all ReduceAddWords workers
		threadPoolReduce.shutdown();
		if (DEBUG)
			System.err.println("Terminating thread pool");
		try {
			while (!threadPoolReduce.awaitTermination(1, TimeUnit.SECONDS)) {
				if (DEBUG)
					System.err.println("Still terminating...");
			}
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		if (DEBUG)
			System.err.println("Terminated all ReduceAddWords threads");
	}

	/**
	 * Gets the results from map tasks
	 * 
	 * @param futures
	 *            - The futures representing map results
	 */
	private static void getMapResults(
			ArrayList<Future<SimpleEntry<String, TreeMap<String, Long>>>> futures) {

		for (Future<SimpleEntry<String, TreeMap<String, Long>>> future : futures) {
			try {
				String docName = future.get().getKey();

				TreeMap<String, Long> chunk = future.get().getValue();
				int index = documentIndices.get(docName).intValue();
				ConcurrentLinkedQueue<TreeMap<String, Long>> tmpqueue = queues
						.get(index);
				tmpqueue.add(chunk);

			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (ExecutionException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

		}
	}

	/**
	 * Initializes queues for reduce tasks
	 */
	private static void initQueues() {
		// Map the filenames with indices
		for (int i = 0; i < indexedDocsNum; ++i)
			documentIndices.put(indexedDocs.get(i), (long) i);

		// Initialize queues lock
		for (int i = 0; i < indexedDocsNum; ++i)
			queuesLock.add(new AtomicInteger());

		// Initialize queues
		for (int i = 0; i < indexedDocsNum; ++i)
			queues.add(new ConcurrentLinkedQueue<TreeMap<String, Long>>());
	}

	/**
	 * Execute map operations
	 * 
	 * @return An array of futures containing hash maps with (word, appearances)
	 *         pairs
	 */
	private static ArrayList<Future<SimpleEntry<String, TreeMap<String, Long>>>> mapOperations() {
		// Create a thread pool
		ExecutorService threadPool = Executors.newFixedThreadPool(NThreads);
		ArrayList<Future<SimpleEntry<String, TreeMap<String, Long>>>> futures = new ArrayList<Future<SimpleEntry<String, TreeMap<String, Long>>>>();

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
					Callable<SimpleEntry<String, TreeMap<String, Long>>> worker = new MapWorker(
							document, i * fragmentSize);
					Future<SimpleEntry<String, TreeMap<String, Long>>> submit = threadPool
							.submit(worker);

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
		if (DEBUG)
			System.err.println("Terminating thread pool");
		try {
			while (!threadPool.awaitTermination(1, TimeUnit.SECONDS)) {
				if (DEBUG)
					System.err.println("Still terminating...");
			}
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		if (DEBUG)
			System.err.println("Terminated all Map threads");
		return futures;
	}

	/**
	 * Creates a StringBuilder with the final output
	 * 
	 * @return The created StringBuilder
	 */
	private static StringBuilder getFinalOutput() {
		StringBuilder sb = new StringBuilder("(");
		for (int i = 0; i < nrCuvCheie; ++i) {
			if (i != nrCuvCheie - 1) {
				sb.append(cuvCheie.get(i) + ", ");
			} else {
				sb.append(cuvCheie.get(i) + ")");
			}
		}
		String keyWords = sb.toString();

		sb = new StringBuilder("Rezultate pentru: " + keyWords + "\n\n");
		if (DEBUG) {
			// Print results
			System.err.println("Rezultate pentru: " + sb.toString() + "\n");
		}

		for (int i = 0, counter = 0; i < indexedDocsNum
				&& counter < numDocumentsX; ++i) {
			if (searchResults.get(i).compareTo("") != 0) {
				++counter;
				if (DEBUG)
					System.err.println(searchResults.get(i));
				sb.append(searchResults.get(i) + "\n");
			}
		}

		if (DEBUG)
			System.err.println("FINAL\n" + sb.toString());
		return sb;
	}

	/**
	 * Writes the final output to file
	 * 
	 * @param sb
	 *            - StringBuilder containing output to be written
	 */
	private static void writeOutput(StringBuilder sb) {
		try {
			BufferedWriter writer = new BufferedWriter(new FileWriter(
					outputFileName));
			writer.write(sb.toString());
			writer.close();
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
				if (DEBUG)
					System.err.println("NrCuvCheie: " + nrCuvCheie);

				if ((line = input.readLine()) != null) {
					String[] words = line.split(" ");
					for (String word : words) {
						cuvCheie.add(word);
					}
				}
				if (DEBUG)
					System.err.println("CuvCheie:\n" + cuvCheie.toString());

				if ((line = input.readLine()) != null) {
					fragmentSize = Integer.parseInt(line);
				}
				if (DEBUG)
					System.err.println("FragmentSize:\t" + fragmentSize);

				if ((line = input.readLine()) != null) {
					mostFrequentN = Integer.parseInt(line);
				}
				if (DEBUG)
					System.err.println("MostFrequentN:\t" + mostFrequentN);

				if ((line = input.readLine()) != null) {
					numDocumentsX = Integer.parseInt(line);
				}
				if (DEBUG)
					System.err.println("NumberOfMostRelevantXDocs:\t"
							+ numDocumentsX);

				if ((line = input.readLine()) != null) {
					indexedDocsNum = Integer.parseInt(line);
				}
				if (DEBUG)
					System.err.println("Number of indexed docs:\t"
							+ indexedDocsNum);

				for (int i = 0; i < indexedDocsNum; ++i) {
					if ((line = input.readLine()) != null) {
						indexedDocs.add(new String(line));
					}
				}
				if (DEBUG)
					System.err.println("Indexed Docs:\n"
							+ indexedDocs.toString());

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
