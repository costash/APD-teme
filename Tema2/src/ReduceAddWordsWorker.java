import java.util.TreeMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * @author Constantin Serban-Radoi 333CA
 * 
 * @category ReduceAddWordsWorker class doing the Reduce by adding words count
 * 
 */
public class ReduceAddWordsWorker implements Runnable {
	private String docName;
	private TreeMap<String, Long> wordsInChunk1;
	private TreeMap<String, Long> wordsInChunk2;
	
	
	public ReduceAddWordsWorker(String docName, TreeMap<String, Long> wordsInChunk1,
			TreeMap<String, Long> wordsInChunk2) {
		this.docName = docName;
		this.wordsInChunk1 = wordsInChunk1;
		this.wordsInChunk2 = wordsInChunk2;
	}

	@Override
	public void run() {
		TreeMap<String, Long> tmpMap;

		tmpMap = wordsInChunk1;
		
		String dbg = new String("\nTempmaps for file" + docName + "\n" + tmpMap + "\n" + wordsInChunk2 + "\n");
		//tmpMap = new TreeMap<String, Long>();
		//tmpMap.putAll(wordsInChunk1);
		for (String w : wordsInChunk2.keySet()) {
			if (!tmpMap.containsKey(w))
				tmpMap.put(w, wordsInChunk2.get(w));
			else {
				String dbgs = new String("Reduce worker for " + docName + " word: {" + w + "} num2: " + wordsInChunk2.get(w) + " num1: " + tmpMap.get(w) + "result: " + (wordsInChunk2.get(w) + tmpMap.get(w)));
				tmpMap.put(w, wordsInChunk2.get(w) + tmpMap.get(w));
				System.err.println(dbgs + " tmp: " + tmpMap.get(w) + "\n");
			}
		}
		dbg += "result" + tmpMap + "\n";
		System.err.print(dbg);

		int index = Main.documentIndices.get(docName).intValue();
		ConcurrentLinkedQueue<TreeMap<String, Long>> tmpqueue = Main.queues.get(index);
		tmpqueue.add(tmpMap);
		
		// Decrement lock for queue
		AtomicInteger lock = Main.queuesLock.get(index);
		lock.decrementAndGet();
	}

}
