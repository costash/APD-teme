import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.AbstractMap.SimpleEntry;
import java.util.ArrayList;
import java.util.TreeMap;
import java.util.concurrent.Callable;


/**
 * @author Constantin Serban-Radoi 333CA
 * 
 * @category MapWorker class doing the map work.
 * 
 */
public class MapWorker implements
		Callable<SimpleEntry<String, TreeMap<String, Long>>> {
	private String docName = new String();
	private int startPos = 0;
	
	private TreeMap<String, Long> treeMap = new TreeMap<String, Long>();
	private SimpleEntry<String, TreeMap<String, Long>> kvp;


	MapWorker(String docName, int startPos) {
		this.docName = docName;
		this.kvp = new SimpleEntry<String, TreeMap<String, Long>>(docName, treeMap);
		this.startPos = startPos;
	}

	@Override
	public SimpleEntry<String, TreeMap<String, Long>> call() throws Exception {
		System.err.println("Working. Worker no "
				+ Thread.currentThread().getId());

		// Get the Array of words for current chunk
		boolean previewsEnded = previewsWordEnded(docName, startPos);
		ArrayList<String> chunkWords = new ArrayList<String>();
		getNextChunk(docName, startPos, chunkWords, previewsEnded);
		
		System.err.println("\n\nWords of " + Thread.currentThread().getName() + 
				"\tstarting at " + startPos + "\n" + chunkWords.toString());

		// Shared between threads: Number of words in a document
		synchronized (Main.wordCount) {
			long sz = (long) chunkWords.size();
			if (!Main.wordCount.containsKey(docName)) {
				Main.wordCount.put(docName, sz);
			}
			else {
				Main.wordCount.put(docName, Main.wordCount.get(docName) + sz);
			}
		}
		
		// Add words to map
		for (String word : chunkWords) {
			if (!treeMap.containsKey(word))
				treeMap.put(word, (long) 1);
			else
				treeMap.put(word, treeMap.get(word) + 1);
		}
		System.err.println("Corresponding map for " + Thread.currentThread().getName() + 
				"\nstarting at " + startPos + "\n" + treeMap.toString() + "\n\n");
		kvp.setValue(treeMap);
		return kvp;
	}

	/**
	 * Checks if the last chunk ended with a delimiter or a letter
	 * 
	 * @param fileName
	 *            - The file where to make the verification
	 * @param startPos
	 *            - The start position of the current chunk
	 * @return - True if previews chunk ended with a delimiter. False otherwise
	 */
	private static boolean previewsWordEnded(String fileName, int startPos) {
		boolean lastWordEnded = false;

		if (startPos == 0)
			return true;

		// Open file
		RandomAccessFile file = null;
		try {
			file = new RandomAccessFile(fileName, "r");
		} catch (FileNotFoundException e1) {
			e1.printStackTrace();
		}

		try {
			// Seek to the chunk position - 1
			file.seek(startPos - 1);
			byte b = file.readByte();
			if (Character.isLetter((char) (b))) {
				lastWordEnded = false;
			} else
				lastWordEnded = true;
			
			file.close();

		} catch (IOException e) {
			e.printStackTrace();
		}
		return lastWordEnded;
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
	private static boolean getNextChunk(String fileName, long pos,
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
			int chunkSize = Main.fragmentSize;
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
			
			file.close();

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

}
