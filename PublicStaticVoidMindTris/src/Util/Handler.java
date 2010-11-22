package Util;

import java.io.IOException;

public interface Handler <C extends Channel>{
	void handle (Data d, C ch) throws IOException;
}
