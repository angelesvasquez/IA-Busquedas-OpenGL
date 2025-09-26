#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>
#include <fstream> 
#include <thread>
#include <chrono>
#include <algorithm>
#include <random>
#include <iomanip>
#include <numeric>
#include <sstream>

using namespace std;
using namespace std::chrono;

std::random_device rd;
std::mt19937 rn(rd());

double euclidiana(pair<int, int> p1, pair<int, int> p2) {
	return sqrt(pow(p2.first - p1.first, 2) + pow(p2.second - p1.second, 2));
}

struct Individuo {
	vector<int> ruta;
	double distanciaTotal; // distancia total de la ruta
	double fi; // aptitud: 1 / distTotal
};

class AgenteViajero {
	int NroC; // Nro ciudades(nodos) 10
	int POBLACION;
	vector<Individuo> poblacion;
	double promedio, suma;
public:
	vector<pair<int, int>> coords; //coordenadas aleatorias en 100x100
	Individuo mejor;
	AgenteViajero(int POP_, int NroC_)
	{
		POBLACION = POP_;
		NroC = NroC_;
		poblacion.resize(POBLACION);
	}
	void generarCiudades() {
		std::uniform_real_distribution<> dist(0, 100);
		coords.resize(NroC);
		for (int i = 0; i < NroC; i++) {
			coords[i] = { dist(rn),dist(rn) };
		}
	}
	
	vector<int> generarRuta() {
		vector<int> rutaN;
		for (int i = NroC - 1; i >= 1; i--) {
			rutaN.push_back(i);
		}
		shuffle(rutaN.begin(), rutaN.end(), rn);
		return rutaN;
	}
	double calcularDistRuta(vector<int> ruta) {
		double total = 0;
		total += euclidiana(coords[0], coords[ruta[0]]);
		for (int i = 0; i < NroC - 2; i++) {
			total += euclidiana(coords[ruta[i]], coords[ruta[i + 1]]);
		}
		total += euclidiana(coords[ruta[NroC - 2]], coords[0]);
		return total;
	}
	void generarPoblacion() {
		for (int i = 0; i < POBLACION; i++) {
			poblacion[i].ruta = generarRuta();
			poblacion[i].distanciaTotal = calcularDistRuta(poblacion[i].ruta);
			poblacion[i].fi = 1 / poblacion[i].distanciaTotal;
			suma += poblacion[i].fi;
			if (i == 0 || mejor.fi < poblacion[i].fi) mejor = poblacion[i];
		}
		promedio = suma / POBLACION;
	}


	void print(int NroC) {
		cout << left;
		int length = round(2.87 * NroC + 29);
		string s(length, '-');

		cout << s << "\n";
		cout << setw(5) << "Ind"
			<< setw(2.8 * NroC) << "Ruta"
			<< setw(14) << "Distancia"
			<< setw(14) << "Aptitud" << endl;
		cout << s << "\n";

		for (int i = 0; i < POBLACION; i++) {
			string rutaStr = "[0 ";
			for (int c : poblacion[i].ruta) {
				rutaStr += to_string(c) + " ";
			}
			rutaStr += "0] ";

			cout << setw(5) << i + 1
				<< setw(2.8 * NroC) << rutaStr
				<< setw(14) << fixed << setprecision(2) << poblacion[i].distanciaTotal
				<< setw(14) << fixed << setprecision(6) << poblacion[i].fi
				<< endl;
		}
		cout << s << "\n";
		cout << "Suma aptitudes:  " << suma << endl;
		cout << "Promedio:        " << promedio << endl;
		cout << "Mejor Aptitud:   " << mejor.fi << endl;
		cout << "Mejor Distancia: " << mejor.distanciaTotal << endl;
		cout << s << "\n\n";
	}
	void exportarEvolucion(const string& nameFile, int generacion) {
		bool fileNew = false;
		ifstream test(nameFile);
		if (!test.good()) fileNew = true;
		test.close();

		ofstream file;
		if (fileNew) {
			file.open(nameFile, ios::out);
			// Crear cabeceras si es archivo nuevo
			file << "Generacion;Promedio;Mejor\n";
		}
		else {
			file.open(nameFile, ios::app);
		}

		if (!file.is_open()) {
			cerr << "No se pudo abrir el archivo de evolución\n";
			return;
		}

		Individuo mejor = obtenerMejor();
		double peor = poblacion[0].fi;
		for (int i = 1; i < POBLACION; i++) {
			if (poblacion[i].fi > peor) peor = poblacion[i].fi;
		}

		file << generacion << ";" << promedio << ";" << mejor.fi << "\n";
		file.close();
	}
	template<class T>
	void paralelizar(int numTH, T tarea) {
		numTH = min(4, POBLACION);  // si es q la poblacion es menor q 4
		vector<thread> THs(numTH);
		for (int i = 0; i < numTH; i++) {
			int start = i * (POBLACION / numTH);
			int end = (i == numTH - 1) ? (POBLACION - 1) : start + (POBLACION / numTH) - 1;

			THs[i] = thread([this, start, end, &tarea]() {
				for (int j = start; j <= end; j++) {
					tarea(j);
				}
				});
		}
		for (auto& h : THs) h.join();
	}

	void calcularFI(Individuo* ind) {
		ind->distanciaTotal = calcularDistRuta(ind->ruta);
		ind->fi = 1 / ind->distanciaTotal;
	}

	void evaluarFIThreads() {
		paralelizar(4, [this](int j) {
			calcularFI(&poblacion[j]);
			});
	}

	Individuo torneo(int nroInd) {
		std::uniform_int_distribution<> dis(0, POBLACION - 1);
		int idx = dis(rn);
		Individuo mejor = poblacion[idx];

		for (int i = 0; i < nroInd - 1; i++) {
			int icand = dis(rn);
			if (poblacion[icand].fi > mejor.fi) {
				mejor = poblacion[icand];
			}
		}
		return mejor;
	}

	Individuo cruzamiento(Individuo& ind1, Individuo& ind2) {
		Individuo h; h.ruta.resize(NroC - 1);
		std::uniform_int_distribution<> dist(0, NroC - 2);
		int c1 = dist(rn), c2 = dist(rn);
		if (c2 < c1) swap(c1, c2);
		vector<bool> usado(NroC, false);

		for (int i = c1; i <= c2; i++) {
			h.ruta[i] = ind1.ruta[i];
			usado[h.ruta[i]] = 1;
		}
		int idx = 0;
		for (int i = 0; i < NroC - 1; i++) {
			int val = ind2.ruta[i];
			if (!usado[val]) {
				while (idx >= c1 && idx <= c2) idx++;
				h.ruta[idx] = val;
				usado[val] = 1;
				idx++;
			}
		}
		return h;
	}
	void mutacion() {
		double probInd = 0.5; // % que mute un individuo
		double probBit = 0.5; // % que mute un nodo
		std::uniform_real_distribution<> disProb(0.0, 1.0);
		std::uniform_int_distribution<> disIdx(0, NroC - 2);
		for (int i = 0; i < POBLACION; i++) {
			if (disProb(rn) < probInd) {
				for (int j = 0; j < NroC - 1; j++) {
					if (disProb(rn) < probBit) {
						int k = disIdx(rn);
						swap(poblacion[i].ruta[j], poblacion[i].ruta[k]);
					}
				}
			}
		}
	}
	Individuo obtenerMejor() {
		Individuo mejor = poblacion[0];
		for (int i = 1; i < POBLACION; i++) {
			suma += poblacion[i].fi;
			if (mejor.fi < poblacion[i].fi) mejor = poblacion[i];
		}
		return mejor;
	}
	bool twoOptFirst(vector<int>& ruta, const vector<pair<int, int>>& coords) {
		for (int i = 1; i < ruta.size() - 1; i++) {
			for (int k = i + 1; k < ruta.size(); k++) {
				int a = ruta[i - 1], b = ruta[i];
				int c = ruta[k], d = ruta[(k + 1) % ruta.size()];

				double antes = euclidiana(coords[a], coords[b]) + euclidiana(coords[c], coords[d]);
				double despues = euclidiana(coords[a], coords[c]) + euclidiana(coords[b], coords[d]);

				if (despues < antes) {
					reverse(ruta.begin() + i, ruta.begin() + k + 1);
					return true;  
				}
			}
		}
		return false;
	}

	void nuevaGeneracion() {
		Individuo mejorAntes = obtenerMejor();
		vector<Individuo> nPoblacion(POBLACION);

		paralelizar(4, [this, &nPoblacion](int j) {
			nPoblacion[j] = torneo(3);
			});
		for (int i = 0; i < POBLACION; i += 2) {
			Individuo h1 = cruzamiento(nPoblacion[i], nPoblacion[i + 1]);
			Individuo h2 = cruzamiento(nPoblacion[i + 1], nPoblacion[i]);
			nPoblacion[i] = h1;
			nPoblacion[i + 1] = h2;
		}
		poblacion = nPoblacion;

		mutacion();
		
		int eliteCount = max(1, POBLACION / 20); // 5%
		for (int i = 0; i < eliteCount; i++) {
			while (twoOptFirst(poblacion[i].ruta, coords));
		}

		evaluarFIThreads();
		int iPeor = 0;
		for (int i = 1; i < POBLACION; i++) {
			if (poblacion[i].fi < poblacion[iPeor].fi) iPeor = i;
		}
		poblacion[iPeor] = mejorAntes;

		actualizarVar();
	
	}
	

	void actualizarVar() {
		suma = promedio = 0;
		for (int i = 0; i < POBLACION; i++) {
			suma += poblacion[i].fi;
			if (mejor.fi < poblacion[i].fi) mejor = poblacion[i];
		}
		promedio = suma / POBLACION;
	}
	void printMatrix() {
		cout << "distancia_matrix = np.array([\n";
		for (int i = 0; i < NroC; i++) {
			cout << "  [";
			for (int j = 0; j < NroC; j++) {
				double d = euclidiana(coords[i], coords[j]);
				cout << d;
				if (j != NroC - 1) cout << ", ";
			}
			cout << "]";
			if (i != NroC - 1) cout << ",";
			cout << "\n";
		}
		cout << "]);\n";

	}
};

AgenteViajero* gAgente;
std::vector<int> gRuta;

void drawText(float x, float y, const std::string& text) {
	glRasterPos2f(x, y);
	for (char c : text) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
	}
}
void drawCircle(float x, float y, float radius, int segments = 20) {
	glBegin(GL_POLYGON);
	for (int i = 0; i < segments; i++) {
		float theta = 2.0f * 3.1415926f * float(i) / float(segments);
		float dx = radius * cosf(theta);
		float dy = radius * sinf(theta);
		glVertex2f(x + dx, y + dy);
	}
	glEnd();
}

void drawAllEdges() {
	glLineWidth(1.0f);
	glColor3f(0.8f, 0.8f, 0.8f); // gris claro

	glBegin(GL_LINES);
	for (int i = 0; i < gAgente->coords.size(); i++) {
		for (int j = i + 1; j < gAgente->coords.size(); j++) {
			auto& c1 = gAgente->coords[i];
			auto& c2 = gAgente->coords[j];
			glVertex2f(static_cast<float>(c1.first), static_cast<float>(c1.second));
			glVertex2f(static_cast<float>(c2.first), static_cast<float>(c2.second));
		}
	}
	glEnd();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT);
	//drawAllEdges();

	glLineWidth(2.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINE_STRIP);
	for (int idx : gRuta) {
		auto& c = gAgente->coords[idx];
		glVertex2f(static_cast<float>(c.first), static_cast<float>(c.second));
	}
	glEnd();

	float cityRadius = 1.0f;
	for (int i = 0; i < gAgente->coords.size(); i++) {
		auto& c = gAgente->coords[i];
		if (i == 0) glColor3f(0.0f, 1.0f, 0.0f); // ciudad inicio verde
		else glColor3f(1.0f, 0.0f, 0.0f);       // otras rojo
		drawCircle(static_cast<float>(c.first), static_cast<float>(c.second), cityRadius);
	}

	for (int i = 0; i < gRuta.size() - 1; i++) {
		auto& c1 = gAgente->coords[gRuta[i]];
		auto& c2 = gAgente->coords[gRuta[i + 1]];
		double dist = euclidiana(c1, c2);
		std::ostringstream ss;
		ss << std::fixed << std::setprecision(2) << dist;

		float xm = (c1.first + c2.first) / 2.0f;
		float ym = (c1.second + c2.second) / 2.0f;
		glColor3f(1.0f, 1.0f, 1.0f);
		drawText(xm, ym, ss.str());
	}

	glutSwapBuffers();
}


int main(int argc, char** argv)
{
	int NroGen = 100, NroC = 40, nroPob = 10;
	AgenteViajero ag(nroPob, NroC); // Nro Poblacion, Nro Ciudades

	ag.generarCiudades();
	ag.generarPoblacion();

	for (int i = 1; i <= NroGen; i++) {
		ag.nuevaGeneracion();
		cout << "Generacion: " << i<<endl;
		ag.print(NroC);
		ag.exportarEvolucion("evolucion.csv", i);
	}

	gAgente = &ag;
	gRuta = ag.mejor.ruta;
	gRuta.insert(gRuta.begin(), 0);
	gRuta.push_back(0);

	glutInit(&argc, argv);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Agente Viajero");
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 100, 0, 100);
	glutDisplayFunc(display);
	glutMainLoop();
}
