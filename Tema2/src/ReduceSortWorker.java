import java.util.TreeMap;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * @author Constantin Serban-Radoi 333CA
 * 
 * @category ReduceSortWorker class doing the Reduce and sort work.
 * 
 */
public class ReduceSortWorker implements Runnable {
	private String docName;
	private TreeMap<String, Long> wordsInChunk1;
	private TreeMap<String, Long> wordsInChunk2;
	
	
	public ReduceSortWorker(String docName, TreeMap<String, Long> wordsInChunk1,
			TreeMap<String, Long> wordsInChunk2) {
		this.docName = docName;
		this.wordsInChunk1 = wordsInChunk1;
		this.wordsInChunk2 = wordsInChunk2;
	}

	@Override
	public void run() {

		/*synchronized (Main.docsFragments) {
			
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
		}*/
		//if (wordsInChunk1.size() < wordsInChunk2.size()) {
		TreeMap<String, Long> tmpMap;

		tmpMap = new TreeMap<String, Long>(wordsInChunk1);
		for (String w : wordsInChunk2.keySet()) {
			if (!tmpMap.containsKey(w))
				tmpMap.put(w, wordsInChunk2.get(w));
			else {
				tmpMap.put(w, wordsInChunk2.get(w) + tmpMap.get(w));
			}
		}
		int index = Main.documentIndices.get(docName).intValue();
		ConcurrentLinkedQueue<TreeMap<String, Long>> tmpqueue = Main.queues.get(index);
		tmpqueue.add(tmpMap);
	}

}
