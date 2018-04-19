package wildsheep.darknet;

public class DarknetUtils {

	private static DarknetUtils api;

	public static DarknetUtils getInstance() {
		synchronized (DarknetUtils.class) {
			System.loadLibrary("yolo_cpp_dll");
			if (api == null) {
				api = new DarknetUtils();
				api.load("lords\\lords.data", "lords\\lords.names", "lords\\tiny-yolo-lords-test.cfg",
						"lords\\tiny-yolo-lords_final.weights");
			}
			return api;
		}
	}

	public native Result[] inference(String pathToRawFile);

	private native boolean load(String datacfg, String name_list_file, String cfgfile, String weightfile);

	public native boolean unload();

}
