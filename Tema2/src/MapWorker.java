
/**
 * @author Constantin Serban-Radoi 333CA
 * 
 * @category MapWorker class doing the map work.
 *
 */
public class MapWorker implements Runnable {

	
	/**
	 * @see java.lang.Runnable#run()
	 */
	@Override
	public void run() {
		// TODO Auto-generated method stub
		System.err.println("Working. ThreadId: " + Thread.currentThread().getId());
	}

}
