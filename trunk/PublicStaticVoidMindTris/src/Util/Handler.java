package Util;

import java.io.IOException;

public interface Handler {
	void handle(byte[] data, Channel ch) throws IOException;
}
