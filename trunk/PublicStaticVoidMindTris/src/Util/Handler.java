package Util;

import IO.*;

import java.io.IOException;

public interface Handler <C extends Channel>{
	void handle (InData in, C ch) throws IOException;
}
