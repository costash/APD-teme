import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;

/**
 * 
 * @author Constantin Serban-Radoi 333CA
 * 
 * @category ReduceReverseSort class doing the Reduce by adding words count
 */
public class ReduceReverseSortWorker implements Runnable {

	private int queueIndex;
	private TreeMap<String,Long> documentWordsMap;
	
	public ReduceReverseSortWorker(int queueIndex, TreeMap<String, Long> documentWordsMap) {
		this.queueIndex = queueIndex;
		this.documentWordsMap = documentWordsMap;
	}
	
	@Override
	public void run() {
		ArrayList<Map.Entry<String, Long>> tmpArr = new ArrayList<Map.Entry<String, Long>>();
		for (Map.Entry<String, Long> entry : documentWordsMap.entrySet()) {
			tmpArr.add(entry);
		}
		
		//System.err.println("\nTemp Array unsorted:\n" + tmpArr);
		Collections.sort(tmpArr, new Comparator<Map.Entry<String, Long>>() {

			@Override
			public int compare(Entry<String, Long> o1, Entry<String, Long> o2) {
				// TODO Auto-generated method stub
				return (int) (o2.getValue() - o1.getValue());
			}
		});
		
		System.err.println("\nFileIdx: " + queueIndex + " file " + Main.indexedDocs.get(queueIndex) + " : Temp Array sorted:\n" + tmpArr);
		// Trim to first N most relevant
		int idx = Main.mostFrequentN - 1;
		int size = tmpArr.size();
		if (idx == size - 1) {
			Main.sortedValues.set(queueIndex, tmpArr);
			return;
		}
		while (idx + 1 < size && tmpArr.get(idx).getValue().compareTo(tmpArr.get(idx + 1).getValue()) == 0) {
			++idx;
		}
		for (int i = size - 1; i >= idx + 1; --i) {
			tmpArr.remove(i);
		}
		
		Main.sortedValues.set(queueIndex, tmpArr);

	}

}
