#ifndef _MAPSGENERATOR_H
#define _MAPSGENERATOR_H

#include <iostream>
#include "Utility.h"
#include "ProvFile.h"
#include "optionsParameters.h"

#include <QCoreApplication>
#include <QPainter>
#include <QFileInfo>
#include <QDir>
#include <QPainterPath>

namespace InsermLibrary
{
	namespace DrawCard
	{
		class mapsGenerator
		{
		public:
			mapsGenerator(int width, int heigth);
			~mapsGenerator();
			//=== TrialMat Drawing Functions
			void trialmatTemplate(std::vector<std::tuple<int, int, int>> codesAndTrials, ProvFile* myprovFile);
			void graduateColorBar(QPainter *painter, int maxValue);
			void drawVerticalZeroLine(QPainter *painter, ProvFile* myprovFile);
			void displayStatsOnMap(QPainter* painter, vec1<PVALUECOORD> significantValue, int electrodeIndex, ProvFile* myprovFile);
			void drawMapTitle(QPainter *painter, std::string title);

			//=== TrialMat Data2Color Functions
			vec2<float> horizontalInterpolation(vec2<float> chanelToInterpol, int interpolationFactor, int idBegTrigg, int nbSubTrials);
			vec2<float> verticalInterpolation(vec2<float> chanelToInterpol, int interpolationFactor);
			void eegData2ColorMap(vec1<int> colorX[512], vec1<int> colorY[512], vec2<float> interpolatedData, float maxValue);
		
			//=== TrialMat Stats2Map
			vec1<int> checkIfNeedDisplayStat(vec1<PVALUECOORD> significantValue, int idCurrentElec);

		private:
			void jetColorMap512(QColor *colorMap);
			void createColorBar(QPainter *painter);
			void defineLineSeparation(QPainter* painter, std::vector<std::tuple<int, int, int>> codesAndTrials, ProvFile* myprovFile);
			void createTimeLegend(QPainter *painter, ProvFile* myprovFile);
			void createTrialsLegend(QPainter* painter, std::vector<std::tuple<int, int, int>> codesAndTrials, ProvFile* myprovFile);

		public:
			QPixmap pixmapTemplate;
			std::vector<QRect> subMatrixes;
			std::vector<int> subMatrixesCodes;
			std::vector<QRect> separationLines;
			QRect MatrixRect;
			QRect colorBarRect;
			QRect fullMap;
			QColor *ColorMapJet = nullptr;
			int Width, Heigth;
		};
	}
}




#endif
