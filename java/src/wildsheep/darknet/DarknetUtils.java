package wildsheep.darknet;

public class DarknetUtils {

	private static DarknetUtils api;

	public static DarknetUtils getInstance() {
		synchronized (DarknetUtils.class) {
			System.loadLibrary("darknet_jni");
			if (api == null) {
				api = new DarknetUtils();
				api.load("cfg\\example.data", "cfg\\example.names", "cfg\\example-tiny-yolo-test-6classes.cfg",
						"cfg\\example-tiny-yolo-6classes.weights");
			}
			return api;
		}
	}

	public native Result[] inference(String pathToRawFile);

	private native boolean load(String datacfg, String name_list_file, String cfgfile, String weightfile);

	public native boolean unload();

}
