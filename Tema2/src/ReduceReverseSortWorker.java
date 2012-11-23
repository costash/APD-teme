import java.math.RoundingMode;
import java.text.DecimalFormat;
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
	private TreeMap<String, Long> documentWordsMap;

	public ReduceReverseSortWorker(int queueIndex,
			TreeMap<String, Long> documentWordsMap) {
		this.queueIndex = queueIndex;
		this.documentWordsMap = documentWordsMap;
	}

	@Override
	public void run() {
		ArrayList<Map.Entry<String, Long>> tmpArr = new ArrayList<Map.Entry<String, Long>>();
		for (Map.Entry<String, Long> entry : documentWordsMap.entrySet()) {
			tmpArr.add(entry);
		}

		if (Main.DEBUG)
			System.err.println("\nTemp Array unsorted:\n" + tmpArr);
		Collections.sort(tmpArr, new Comparator<Map.Entry<String, Long>>() {

			@Override
			public int compare(Entry<String, Long> o1, Entry<String, Long> o2) {
				// TODO Auto-generated method stub
				return (int) (o2.getValue() - o1.getValue());
			}
		});

		if (Main.DEBUG)
			System.err.println("\nFileIdx: " + queueIndex + " file "
					+ Main.indexedDocs.get(queueIndex)
					+ " : Temp Array sorted:\n" + tmpArr);
		// Trim to first N most relevant
		int idx = Main.mostFrequentN - 1;
		int size = tmpArr.size();
		if (idx == size - 1) {
			Main.sortedValues.set(queueIndex, tmpArr);
			return;
		}
		while (idx + 1 < size
				&& tmpArr.get(idx).getValue()
						.compareTo(tmpArr.get(idx + 1).getValue()) == 0) {
			++idx;
		}
		for (int i = size - 1; i >= idx + 1; --i) {
			tmpArr.remove(i);
		}

		boolean keyWordMissing = checkWordMissing(tmpArr);

		// All keywords have been found for this document
		if (keyWordMissing == false) {
			computeFinalOutput();
		}

		Main.sortedValues.set(queueIndex, tmpArr);

	}

	/**
	 * Checks if any word from query is missing
	 * 
	 * @param tmpArr
	 *            - The computed array of words
	 * @return True if any word is missing, False otherwise
	 */
	private boolean checkWordMissing(ArrayList<Map.Entry<String, Long>> tmpArr) {
		boolean keyWordMissing = false;
		for (int i = 0; i < Main.nrCuvCheie; ++i) {
			boolean keyWordFound = false;
			for (int j = 0; j < tmpArr.size(); ++j)
				if (tmpArr.get(j).getKey().compareTo(Main.cuvCheie.get(i)) == 0)
					keyWordFound = true;
			if (keyWordFound == false) {
				keyWordMissing = true;
				break;
			}
		}
		return keyWordMissing;
	}

	/**
	 * Computes the final output
	 */
	private void computeFinalOutput() {
		StringBuilder sb = new StringBuilder(Main.indexedDocs.get(queueIndex)
				+ " (");
		for (int i = 0; i < Main.nrCuvCheie; ++i) {
			int totalWords = Main.wordCount.get(
					Main.indexedDocs.get(queueIndex)).intValue();
			float frequency = ((float) documentWordsMap.get(Main.cuvCheie
					.get(i)) / (float) totalWords) * 100;

			if (Main.DEBUG)
				System.err.println("Frecv pt " + Main.cuvCheie.get(i)
						+ " in doc " + Main.indexedDocs.get(queueIndex) + ": "
						+ frequency + "total nr" + totalWords);

			DecimalFormat df = new DecimalFormat("0.00");
			df.setRoundingMode(RoundingMode.FLOOR);
			if (Main.DEBUG)
				System.err.println(df.format(frequency));

			if (i != Main.nrCuvCheie - 1) {
				sb.append(df.format(frequency) + ", ");
			} else {
				sb.append(df.format(frequency) + ")");
			}
			if (Main.DEBUG)
				System.err.println(sb.toString());

			Main.searchResults.set(queueIndex, sb.toString());
		}
	}

}
