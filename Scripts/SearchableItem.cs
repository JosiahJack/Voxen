
	void Start () {
		int numRandomGeneratedItems = 0;
		if (generateContents && !generationDone) {
			// Generate random contents once
			int tempInt = 100;
			for(int i=0;i<randomItem.Length;i++) {
				tempInt = random_range(0,100); // generate even distribution random value from 0 to 100, e.g. 35
				if (randomItemDropChance[i] <= 0) continue; // next!
				if (tempInt <= randomItemDropChance[i]) {
					contEng_Global->instances[numRandomGeneratedItems] = randomItem[i]; // ok item is now present
					numRandomGeneratedItems++;
					if (numRandomGeneratedItems>maxRandomItems) break; // all done we have all our contents
				}
			}
			generationDone = true;
		}
	}

	public void ResetSearchable(bool wipeContents) {
		searchableInUse = false;
		if (wipeContents) {
			contEng_Global->instances[0] = -1;
			contEng_Global->instances[1] = -1;
			contEng_Global->instances[2] = -1;
			contEng_Global->instances[3] = -1;
			customIndex[0] = -1;
			customIndex[1] = -1;
			customIndex[2] = -1;
			customIndex[3] = -1;
		}
	}
}
