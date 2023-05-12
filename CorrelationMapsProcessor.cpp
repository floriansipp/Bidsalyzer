#include "CorrelationMapsProcessor.h"
#include "../../Framework/Framework/Pearson.h"
#include <QPainter>
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock

void InsermLibrary::CorrelationMapsProcessor::Process(eegContainer* myeegContainer, std::string freqFolder, std::string ptsFilePath)
{
	//Hardcoded stuff, look for parameters later
	int halfWindowSizeInSeconds = 15;
	//=== end hardcoded stuff

	int ElectrodeCount = myeegContainer->elanFrequencyBand[0]->ElectrodeCount();
	int stepInSample = round(myeegContainer->elanFrequencyBand[0]->SamplingFrequency() * halfWindowSizeInSeconds);
	std::vector<int> windowsCenter = DefineCorrelationWindowsCenter(stepInSample, myeegContainer->elanFrequencyBand[0]->NumberOfSamples());
	int TriggerCount = windowsCenter.size();

	//== get Bloc of eeg data we want to display center around events
	vec3<float> eegData3D = vec3<float>(TriggerCount, vec2<float>(ElectrodeCount, vec1<float>(2 * stepInSample)));
	std::vector<std::vector<float>> currentData = myeegContainer->elanFrequencyBand[0]->Data(EEGFormat::DataConverterType::Analog);
	for (int i = 0; i < TriggerCount; i++)
	{
		for (int j = 0; j < (2 * stepInSample); j++)
		{
			int beginTime = windowsCenter[i] - stepInSample;

			for (int k = 0; k < ElectrodeCount; k++)
			{
				//to prevent issue in case the first event has been recorded realy quick
				if (beginTime + j < 0)
					eegData3D[i][k][j] = 0;
				else
					eegData3D[i][k][j] = currentData[k][beginTime + j];
			}
		}
	}

	//== Compute circular coordinates
	std::vector<std::vector<float>> dist = (ptsFilePath != "") ? ComputeElectrodesDistancesFromPts(myeegContainer, ptsFilePath) : ComputeElectrodesDistances(myeegContainer);

	//== Compute surrogate (nb = 1000000 , hardcoded for now)
	float s_rmax = ComputeSurrogate(ElectrodeCount, TriggerCount, 1000000, dist, eegData3D);
	float s_rmin = -1 * s_rmax;

	//== Computecorrelations
	std::vector<std::vector<float>> bigCorrePlus = std::vector<std::vector<float>>(ElectrodeCount, std::vector<float>(ElectrodeCount));
	std::vector<std::vector<float>> bigCorreMinus = std::vector<std::vector<float>>(ElectrodeCount, std::vector<float>(ElectrodeCount));
	for (int ii = 0; ii < TriggerCount; ii++)
	{
		for (int i = 0; i < ElectrodeCount; i++)
		{
			for (int j = 0; j < ElectrodeCount; j++)
			{
				if (i == j)
				{
					bigCorrePlus[i][j] = 1;
					bigCorreMinus[i][j] = 1;
					continue;
				}

				float corre = Framework::Calculations::Stats::Correlation::pearsonCoefficient(eegData3D[ii][i], eegData3D[ii][j]);
				bigCorrePlus[i][j] += (corre > s_rmax) ? (1.0f / TriggerCount) : 0;
				bigCorreMinus[i][j] += (corre < s_rmin) ? (1.0f / TriggerCount) : 0;
			}
		}
	}

	std::string outputFilePath = DefineMapPath(freqFolder, myeegContainer->DownsamplingFactor(), halfWindowSizeInSeconds * 2);
	//==================================
	int width = 2400;
	int height = 1200;

	QPixmap* pixmapChanel = new QPixmap(width, height);
	pixmapChanel->fill(QColor(Qt::white));
	QPainter* painterChanel = new QPainter(pixmapChanel);

	int offset = width / 2;
	DrawCorrelationCircle(painterChanel, myeegContainer, width / 2, height / 2, 0); //negativ correlation area
	DrawCorrelationCircle(painterChanel, myeegContainer, width / 2, height / 2, offset); //positiv correlation area

	DrawCorrelationOnCircle(painterChanel, height / 2, 0, dist, bigCorreMinus); //negativ correlations
	DrawCorrelationOnCircle(painterChanel, height / 2, offset, dist, bigCorrePlus); //positiv correlations

	pixmapChanel->save(outputFilePath.c_str(), "JPG");
	deleteAndNullify1D(painterChanel);
	deleteAndNullify1D(pixmapChanel);
}

std::string InsermLibrary::CorrelationMapsProcessor::DefineMapPath(std::string freqFolder, int dsSampFreq, int windowSizeInSec)
{
	vec1<std::string> pathSplit = split<std::string>(freqFolder, "/");
	return std::string(freqFolder + "/" + pathSplit[pathSplit.size() - 1] + "_ds" + std::to_string(dsSampFreq) + "_sm0_ccircle_w" + std::to_string(windowSizeInSec) + "s.jpg");
}

//Define correlation Windows
//from the file size and halfwindowsize, determine the centers (in sample)
//of all the windows that will be considered for correlation computation
std::vector<int> InsermLibrary::CorrelationMapsProcessor::DefineCorrelationWindowsCenter(int halfWindowSizeInSample, int fileSizeInSample)
{
	std::vector<int> windowsCenter;

	windowsCenter.push_back(halfWindowSizeInSample);
	while (windowsCenter[windowsCenter.size() - 1] + (2 * halfWindowSizeInSample) < fileSizeInSample)
	{
		windowsCenter.push_back(windowsCenter[windowsCenter.size() - 1] + halfWindowSizeInSample);
	}

	//We remove the two extreme border windows to avoid border effects
	windowsCenter.erase(windowsCenter.begin() + (windowsCenter.size() - 1));
	windowsCenter.erase(windowsCenter.begin());

	return windowsCenter;
}

std::vector<std::vector<float>> InsermLibrary::CorrelationMapsProcessor::ComputeElectrodesDistances(eegContainer* myeegContainer)
{
	std::vector<std::vector<float>> dist = std::vector<std::vector<float>>(myeegContainer->elanFrequencyBand[0]->ElectrodeCount(), std::vector<float>(myeegContainer->elanFrequencyBand[0]->ElectrodeCount()));
	for (int i = 0; i < myeegContainer->elanFrequencyBand[0]->ElectrodeCount(); i++)
	{
		for (int j = 0; j < myeegContainer->elanFrequencyBand[0]->ElectrodeCount(); j++)
		{
			std::string mainLabel = myeegContainer->elanFrequencyBand[0]->Electrode(i)->Label();
			std::string secondaryLabel = myeegContainer->elanFrequencyBand[0]->Electrode(j)->Label();

			//from InsermLibrary::eegContainer::GetIndexFromElectrodeLabel
			int goodId = -1;
			for (int k = 0; k < mainLabel.size(); k++)
			{
				if (isdigit(mainLabel[k]) && mainLabel[k] != 0)
				{
					goodId = k;
					break;
				}
			}
			std::string subLabel = mainLabel.substr(0, goodId);
			int contactIndex = stoi(mainLabel.substr(goodId, mainLabel.size()));

			goodId = -1;
			for (int k = 0; k < secondaryLabel.size(); k++)
			{
				if (isdigit(secondaryLabel[k]) && secondaryLabel[k] != 0)
				{
					goodId = k;
					break;
				}
			}
			std::string subLabel2 = secondaryLabel.substr(0, goodId);
			int contactIndex2 = stoi(secondaryLabel.substr(goodId, secondaryLabel.size()));

			dist[i][j] = (subLabel == subLabel2) ? contactIndex2 - contactIndex : 30;
		}
	}
	return dist;
}

std::vector<std::vector<float>> InsermLibrary::CorrelationMapsProcessor::ComputeElectrodesDistancesFromPts(eegContainer* myeegContainer, std::string ptsFilePath)
{
	std::vector<std::string> rawFile = readTxtFile(ptsFilePath);
	int electrodeCount = QString::fromStdString(rawFile[2]).toInt();

	std::vector<std::string> Label = std::vector<std::string>(electrodeCount);
	std::vector<float> x = std::vector<float>(electrodeCount);
	std::vector<float> y = std::vector<float>(electrodeCount);
	std::vector<float> z = std::vector<float>(electrodeCount);

	for (int i = 0; i < electrodeCount; i++)
	{
		std::vector<std::string> rawLine = InsermLibrary::split<std::string>(rawFile[3 + i], "\t ");
		Label[i] = rawLine[0];
		x[i] = QString::fromStdString(rawLine[1]).toFloat();
		y[i] = QString::fromStdString(rawLine[2]).toFloat();
		z[i] = QString::fromStdString(rawLine[3]).toFloat();
	}

	std::vector<std::vector<float>> dist = std::vector<std::vector<float>>(myeegContainer->elanFrequencyBand[0]->ElectrodeCount(), std::vector<float>(myeegContainer->elanFrequencyBand[0]->ElectrodeCount()));
	for (int i = 0; i < myeegContainer->elanFrequencyBand[0]->ElectrodeCount(); i++)
	{
		for (int j = 0; j < myeegContainer->elanFrequencyBand[0]->ElectrodeCount(); j++)
		{
			std::string mainLabel = myeegContainer->elanFrequencyBand[0]->Electrode(i)->Label();
			std::string mainLabel2 = mainLabel;
			std::replace(mainLabel2.begin(), mainLabel2.end(), '\'', 'p');

			std::string secondaryLabel = myeegContainer->elanFrequencyBand[0]->Electrode(j)->Label();
			std::string secondaryLabel2 = secondaryLabel;
			std::replace(secondaryLabel2.begin(), secondaryLabel2.end(), '\'', 'p');

			std::vector<std::string>::iterator it = std::find(Label.begin(), Label.end(), mainLabel2);
			int index = std::distance(Label.begin(), it);

			std::vector<std::string>::iterator it2 = std::find(Label.begin(), Label.end(), secondaryLabel2);
			int index2 = std::distance(Label.begin(), it2);

			float distt = 0.0f;
			//if one of the indexes is not found we consider coordinates to be 0,0,0
			if (index2 == Label.size())
			{
				distt = sqrtf((0 - x[index]) * (0 - x[index]) + (0 - y[index]) * (0 - y[index]) + (0 - z[index]) * (0 - z[index]));
			}
			else if (index == Label.size())
			{
				distt = sqrtf((x[index2] * x[index2]) + (y[index2] * y[index2]) + (z[index2] * z[index2]));
			}
			else
			{
				distt = sqrtf((x[index2] - x[index]) * (x[index2] - x[index]) + (y[index2] - y[index]) * (y[index2] - y[index]) + (z[index2] - z[index]) * (z[index2] - z[index]));
			}

			dist[i][j] = distt;
		}
	}

	return dist;
}

/// <summary>
/// Compute surrogates
/// </summary>
/// <param name="electrodeCount"></param>
/// <param name="triggerCount"></param>
/// <param name="surrogateCount"></param>
/// <param name="distances"></param>
/// <param name="eegData"></param>
/// <returns>the surrogate value at 99.99%</returns>
/// 

float InsermLibrary::CorrelationMapsProcessor::ComputeSurrogate(int electrodeCount, int triggerCount, int surrogateCount, vec2<float> distances, vec3<float> eegData)
{
	std::vector<float> surrogates = std::vector<float>(surrogateCount);
#pragma omp parallel for
	for (int ii = 0; ii < surrogateCount; ii++)
	{
		std::vector<int> rand;
		for (int i = 0; i < electrodeCount; i++)
		{
			rand.push_back(i);
		}

		// obtain a time-based seed and shuffle
		unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::shuffle(rand.begin(), rand.end(), std::default_random_engine(seed));

		int seedIndex1 = rand[0];
		//==================================
		std::vector<int> matches;
		for (int i = 0; i < distances[seedIndex1].size(); i++)
		{
			if (distances[seedIndex1][i] >= 30)
			{
				matches.push_back(i);
			}
		}

		unsigned int seed2 = std::chrono::system_clock::now().time_since_epoch().count();
		std::shuffle(matches.begin(), matches.end(), std::default_random_engine(seed2));

		int seedIndex2 = matches[0];
		//==================================
		std::vector<int> eventRand;
		for (int i = 0; i < round(triggerCount / 3); i++)
		{
			eventRand.push_back(i);
		}

		unsigned int seed3 = std::chrono::system_clock::now().time_since_epoch().count();
		std::shuffle(eventRand.begin(), eventRand.end(), std::default_random_engine(seed3));

		int seedIndexEvent = eventRand[0];
		int seedIndexEvent2 = (triggerCount - 1) - seedIndexEvent;
		//==================================

		surrogates[ii] = Framework::Calculations::Stats::Correlation::pearsonCoefficient(eegData[seedIndexEvent][seedIndex1], eegData[seedIndexEvent2][seedIndex2]);
	}

	std::vector<float> surrogatesAbs = std::vector<float>(surrogates);
	for (int i = 0; i < surrogateCount; i++)
	{
		surrogatesAbs[i] = abs(surrogates[i]);
	}
	std::sort(surrogatesAbs.begin(), surrogatesAbs.end());

	int index = round(0.9999f * surrogateCount);
	return surrogatesAbs[index];
}

void InsermLibrary::CorrelationMapsProcessor::DrawCorrelationCircle(QPainter* painterChanel, eegContainer* myeegContainer, int halfwidth, int halfheight, int offset)
{
	//#define PI 3.14159265f
	int radius = halfheight - 30;
	int ElectrodeCount = myeegContainer->elanFrequencyBand[0]->ElectrodeCount();
	std::string elecNameStringTemp = "%#";
	for (int i = 0; i < ElectrodeCount; i++)
	{
		QString label = QString::fromStdString(myeegContainer->elanFrequencyBand[0]->Electrode(i)->Label());
		QColor color = GetColorFromLabel(label.toStdString(), elecNameStringTemp);

		//elec label
		float angle = (2 * 3.14159265f * i) / ElectrodeCount;
		float x = radius * cos(angle);
		float y = radius * sin(angle);
		painterChanel->drawText(QPoint(offset + halfheight + x, halfheight - y), label);

		//== correlation circle point
		x = (radius - 60) * cos(angle);
		y = (radius - 60) * sin(angle);
		painterChanel->setPen(color);
		painterChanel->drawEllipse(QPoint(offset + halfheight + x, halfheight - y), 8, 8);
		painterChanel->setPen(Qt::black);
	}

	m_colorId = -1;
}

void InsermLibrary::CorrelationMapsProcessor::DrawCorrelationOnCircle(QPainter* painterChanel, int halfheight, int offset, std::vector<std::vector<float>> dist, std::vector<std::vector<float>> corre)
{
	int ElectrodeCount = corre.size();
	int radius = halfheight - 30;
	int s_minpct_toshow = 10;
	for (int i = 0; i < ElectrodeCount - 1; i++)
	{
		float angle = (2 * 3.14159265f * i) / ElectrodeCount;
		float x = (radius - 60) * cos(angle);
		float y = (radius - 60) * sin(angle);
		for (int j = 1; j < ElectrodeCount; j++)
		{
			float angle2 = (2 * 3.14159265f * j) / ElectrodeCount;
			float x2 = (radius - 60) * cos(angle2);
			float y2 = (radius - 60) * sin(angle2);

			if (dist[i][j] > 25)
			{
				float width = floor(((float)100 / s_minpct_toshow) * corre[i][j]);
				if (width > 0.0f)
				{
					QPoint p1(offset + halfheight + x, halfheight - y);
					QPoint p2(offset + halfheight + x2, halfheight - y2);
					painterChanel->drawLine(p1, p2);
				}
			}
		}
	}
}

QColor InsermLibrary::CorrelationMapsProcessor::GetColorFromLabel(std::string label, std::string& memoryLabel)
{
	QColor colors[]{ QColor(Qt::red), QColor(Qt::blue), QColor(Qt::darkYellow), QColor(Qt::green), QColor(Qt::gray), QColor(Qt::cyan), QColor(Qt::black), QColor(Qt::magenta) };

	std::string result = "";
	int resId = -1;

	int goodId = GetIndexFromElectrodeLabel(label);

	if (goodId != -1)
	{
		result = label.substr(0, goodId);
		resId = stoi(label.substr(goodId, label.size()));
	}
	else
	{
		result = label;
	}

	if (result.find(memoryLabel) != std::string::npos && (result.length() == memoryLabel.length()))
	{
		return colors[m_colorId];
	}
	else
	{
		memoryLabel = result;
		m_colorId = (m_colorId + 1) % 8;
		return colors[m_colorId];
	}
}

int InsermLibrary::CorrelationMapsProcessor::GetIndexFromElectrodeLabel(std::string myString)
{
	for (int j = 0; j < myString.size(); j++)
	{
		if (isdigit(myString[j]) && myString[j] != 0)
		{
			return j;
		}
	}
	return -1;
}