#ifndef _BARSPLOTSGENERATOR_H
#define _BARSPLOTSGENERATOR_H

#include <iostream>
#include <algorithm>
#include "Utility.h"
#include "ProvFile.h"
#include "eegContainer.h"
#include "TriggerContainer.h"
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include "optionsParameters.h"

namespace InsermLibrary
{
	namespace DrawbarsPlots
	{
		class baseCanvas
		{
		public:
			baseCanvas(ProvFile* myprovFile, int width, int heigth);
			~baseCanvas();
		private:
			void drawTemplate(ProvFile* myprovFile);
		public:
			int nbElecPerFigure = 3;
			int nbSite = 20;
			int nbColLegend = 0;
			int Width = 0, Heigth = 0;
			QPixmap pixmapTemplate;
		};

		class drawBars : public baseCanvas
		{
		public:
			drawBars(ProvFile* myprovFile, std::string outputFolder, QSize size);
			~drawBars();
			void drawDataOnTemplate(vec3<float> &bigData, TriggerContainer* triggerContainer, vec1<PVALUECOORD> significantValue, eegContainer* myeegContainer);
			QString createPicPath(std::string picFolder, eegContainer* myeegContainer, int idElec);
		public:
			ProvFile* myprovFile = nullptr;
			std::string picOutputFolder = "";
		};

		class drawPlots : public baseCanvas
		{
		public:
			drawPlots(ProvFile* myprovFile, std::string outputFolder, QSize size);
			~drawPlots();
			void drawDataOnTemplate(vec3<float> &bigData, TriggerContainer* triggerContainer, eegContainer* myeegContainer, int card2Draw);
			QString createPicPath(std::string picFolder, eegContainer* myeegContainer, int cards2Draw, int nbFigureDrawn);
		public:
			ProvFile* myprovFile = nullptr;
			std::string picOutputFolder = "";
		};
	}
}


#endif
