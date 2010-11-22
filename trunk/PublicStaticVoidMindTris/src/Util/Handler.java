package Util;

import java.io.IOException;

public interface Handler <C extends Channel>{
	void handle(byte[] data, C ch) throws IOException;
}
