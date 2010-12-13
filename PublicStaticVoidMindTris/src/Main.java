import java.security.Security;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

import Client.CreateClient;

public class Main {

	public static void main(String[] args) {
		Security.addProvider(new BouncyCastleProvider());
		
		CreateClient.main(args);
	}

}
