import java.util.TreeMap;

/**
 * @author Constantin Serban-Radoi 333CA
 * 
 * @category ReduceSortWorker class doing the Reduce and sort work.
 * 
 */
public class ReduceSortWorker implements Runnable {
	private String docName;
	private TreeMap<String, Long> wordsInChunk;
	
	
	public ReduceSortWorker(String docName, TreeMap<String, Long> wordsInChunk) {
		this.docName = docName;
		this.wordsInChunk = wordsInChunk;
	}

	@Override
	public void run() {

		synchronized (Main.docsFragments) {
			
			TreeMap<String, Long> tm = Main.docsFragments.get(docName);
			if (tm == null)
				tm = new TreeMap<String, Long>();
			for (String w : wordsInChunk.keySet()) {
				if (!tm.containsKey(w))
					tm.put(w, wordsInChunk.get(w));
				else {
					tm.put(w, tm.get(w) + wordsInChunk.get(w));
				}
			}
			
			Main.docsFragments.put(docName, tm);
		}
	}

}
