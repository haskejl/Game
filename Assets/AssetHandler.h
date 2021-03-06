/*
 * AssetHandler.h
 *
 *  Created on: May 4, 2019
 *      Author: user1
 */

#ifndef ASSETHANDLER_H_
#define ASSETHANDLER_H_

#include "ModelHandler.h"

class AssetHandler {
public:
	AssetHandler();
	virtual ~AssetHandler();

	void loadAssets();
	void drawAssets();

private:
	ModelHandler mh;
};

#endif /* ASSETHANDLER_H_ */
