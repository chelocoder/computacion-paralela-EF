#include <iostream>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <vector>
#include <string>
#include <omp.h>

using namespace std;
using namespace chrono;

// ------------------------------------------------------------
// EVALUACION FINAL - COMPUTACION PARALELA Y DISTRIBUIDA
// Caso: procesamiento masivo de datos climaticos asociados
// a un evento extremo de El Nino en el Peru.
//
// Autor: Juan Marcelo Chamorro Avendano
// ------------------------------------------------------------

const long long N = 100000000LL; // 100 millones de mediciones
const int REPETICIONES = 5;

// Politicas de scheduling compatibles con MSVC
enum class Politica {
    Static,
    Dynamic,
    Guided
};

struct Resultado {
    long long sumaTemperatura;
    long long sumaPrecipitacion;
    long long sumaHumedad;
    long long sumaCaudal;
    long long alertas;
};

// ------------------------------------------------------------
// Generador determinista de datos climaticos
// ------------------------------------------------------------
inline unsigned int hashDato(unsigned long long x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;

    return static_cast<unsigned int>(x);
}

// ------------------------------------------------------------
// Procesamiento de una medicion climatica
// ------------------------------------------------------------
inline void procesarDato(
    long long i,
    long long& temperatura,
    long long& precipitacion,
    long long& humedad,
    long long& caudal,
    long long& alerta
) {
    unsigned int h1 = hashDato(i);
    unsigned int h2 = hashDato(i + 1234567ULL);
    unsigned int h3 = hashDato(i + 7654321ULL);

    // Temperatura entre 10.0 y 39.9 C
    // Se almacena multiplicada por 10
    temperatura = 100 + (h1 % 300);

    // Precipitacion entre 0.0 y 199.9 mm
    // Se almacena multiplicada por 10
    precipitacion = h2 % 2000;

    // Humedad entre 30 y 99 %
    humedad = 30 + (h3 % 70);

    // Caudal entre 50 y 1049 m3/s
    caudal = 50 + ((h1 ^ h2) % 1000);

    // Regla simple para detectar una condicion climatica critica
    alerta =
        (precipitacion > 1500 &&
         humedad > 85 &&
         caudal > 800)
        ? 1
        : 0;
}

// ------------------------------------------------------------
// BASELINE SECUENCIAL
// ------------------------------------------------------------
Resultado ejecutarSecuencial() {

    Resultado r{0, 0, 0, 0, 0};

    for (long long i = 0; i < N; ++i) {

        long long temperatura;
        long long precipitacion;
        long long humedad;
        long long caudal;
        long long alerta;

        procesarDato(
            i,
            temperatura,
            precipitacion,
            humedad,
            caudal,
            alerta
        );

        r.sumaTemperatura += temperatura;
        r.sumaPrecipitacion += precipitacion;
        r.sumaHumedad += humedad;
        r.sumaCaudal += caudal;
        r.alertas += alerta;
    }

    return r;
}

// ------------------------------------------------------------
// VERSION PARALELA OPENMP
// ------------------------------------------------------------
Resultado ejecutarParalelo(
    int threads,
    Politica politica
) {
    omp_set_num_threads(threads);

    long long sumaTemperatura = 0;
    long long sumaPrecipitacion = 0;
    long long sumaHumedad = 0;
    long long sumaCaudal = 0;
    long long alertas = 0;

    // --------------------------------------------------------
    // STATIC
    // --------------------------------------------------------
    if (politica == Politica::Static) {

        #pragma omp parallel for schedule(static, 10000) \
            reduction(+:sumaTemperatura, sumaPrecipitacion, sumaHumedad, sumaCaudal, alertas)

        for (long long i = 0; i < N; ++i) {

            long long temperatura;
            long long precipitacion;
            long long humedad;
            long long caudal;
            long long alerta;

            procesarDato(
                i,
                temperatura,
                precipitacion,
                humedad,
                caudal,
                alerta
            );

            sumaTemperatura += temperatura;
            sumaPrecipitacion += precipitacion;
            sumaHumedad += humedad;
            sumaCaudal += caudal;
            alertas += alerta;
        }
    }

    // --------------------------------------------------------
    // DYNAMIC
    // --------------------------------------------------------
    else if (politica == Politica::Dynamic) {

        #pragma omp parallel for schedule(dynamic, 10000) \
            reduction(+:sumaTemperatura, sumaPrecipitacion, sumaHumedad, sumaCaudal, alertas)

        for (long long i = 0; i < N; ++i) {

            long long temperatura;
            long long precipitacion;
            long long humedad;
            long long caudal;
            long long alerta;

            procesarDato(
                i,
                temperatura,
                precipitacion,
                humedad,
                caudal,
                alerta
            );

            sumaTemperatura += temperatura;
            sumaPrecipitacion += precipitacion;
            sumaHumedad += humedad;
            sumaCaudal += caudal;
            alertas += alerta;
        }
    }

    // --------------------------------------------------------
    // GUIDED
    // --------------------------------------------------------
    else {

        #pragma omp parallel for schedule(guided, 10000) \
            reduction(+:sumaTemperatura, sumaPrecipitacion, sumaHumedad, sumaCaudal, alertas)

        for (long long i = 0; i < N; ++i) {

            long long temperatura;
            long long precipitacion;
            long long humedad;
            long long caudal;
            long long alerta;

            procesarDato(
                i,
                temperatura,
                precipitacion,
                humedad,
                caudal,
                alerta
            );

            sumaTemperatura += temperatura;
            sumaPrecipitacion += precipitacion;
            sumaHumedad += humedad;
            sumaCaudal += caudal;
            alertas += alerta;
        }
    }

    return {
        sumaTemperatura,
        sumaPrecipitacion,
        sumaHumedad,
        sumaCaudal,
        alertas
    };
}

// ------------------------------------------------------------
// Verificacion de correctitud
// ------------------------------------------------------------
bool iguales(
    const Resultado& a,
    const Resultado& b
) {
    return
        a.sumaTemperatura == b.sumaTemperatura &&
        a.sumaPrecipitacion == b.sumaPrecipitacion &&
        a.sumaHumedad == b.sumaHumedad &&
        a.sumaCaudal == b.sumaCaudal &&
        a.alertas == b.alertas;
}

// ------------------------------------------------------------
// Nombre de politica
// ------------------------------------------------------------
string nombrePolitica(Politica politica) {

    if (politica == Politica::Static)
        return "static";

    if (politica == Politica::Dynamic)
        return "dynamic";

    return "guided";
}

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
int main() {

    cout << fixed << setprecision(6);

    cout << "============================================================\n";
    cout << " PROCESAMIENTO MASIVO DE DATOS CLIMATICOS - PERU\n";
    cout << " Evaluacion Final - Computacion Paralela y Distribuida\n";
    cout << "============================================================\n\n";

    cout << "Mediciones simuladas : " << N << "\n";
    cout << "Repeticiones          : " << REPETICIONES << "\n";
    cout << "Procesadores OpenMP   : " << omp_get_num_procs() << "\n";
    cout << "Max threads OpenMP    : " << omp_get_max_threads() << "\n\n";

    // ========================================================
    // BASELINE SECUENCIAL
    // ========================================================

    cout << "BASELINE SECUENCIAL\n";
    cout << "------------------------------------------------------------\n";

    double tiempoSerialTotal = 0.0;

    Resultado baseline{};

    for (int repeticion = 1;
         repeticion <= REPETICIONES;
         ++repeticion) {

        auto inicio =
            high_resolution_clock::now();

        Resultado resultado =
            ejecutarSecuencial();

        auto fin =
            high_resolution_clock::now();

        double tiempo =
            duration<double>(fin - inicio).count();

        tiempoSerialTotal += tiempo;

        baseline = resultado;

        cout << "Corrida "
             << repeticion
             << ": "
             << tiempo
             << " s\n";
    }

    double T1 =
        tiempoSerialTotal / REPETICIONES;

    cout << "\nTiempo promedio serial T1 = "
         << T1
         << " s\n";

    cout << "\nResultado baseline:\n";

    cout << "Temperatura promedio = "
         << (baseline.sumaTemperatura / (double)N) / 10.0
         << " C\n";

    cout << "Precipitacion promedio = "
         << (baseline.sumaPrecipitacion / (double)N) / 10.0
         << " mm\n";

    cout << "Humedad promedio = "
         << baseline.sumaHumedad / (double)N
         << " %\n";

    cout << "Caudal promedio = "
         << baseline.sumaCaudal / (double)N
         << " m3/s\n";

    cout << "Alertas detectadas = "
         << baseline.alertas
         << "\n\n";

    // ========================================================
    // ARCHIVO CSV
    // ========================================================

    ofstream csv("resultados.csv");

    csv
        << "politica,"
        << "threads,"
        << "tiempo_promedio,"
        << "speedup,"
        << "eficiencia,"
        << "correcto\n";

    vector<int> threads = {
        1,
        2,
        4,
        8
    };

    vector<Politica> politicas = {
        Politica::Static,
        Politica::Dynamic,
        Politica::Guided
    };

    // ========================================================
    // EXPERIMENTOS OPENMP
    // ========================================================

    for (Politica politica : politicas) {

        cout
            << "\n============================================================\n";

        cout
            << "SCHEDULE: "
            << nombrePolitica(politica)
            << "\n";

        cout
            << "============================================================\n";

        for (int p : threads) {

            double tiempoTotal = 0.0;
            bool correcto = true;

            cout
                << "\nThreads = "
                << p
                << "\n";

            for (int repeticion = 1;
                 repeticion <= REPETICIONES;
                 ++repeticion) {

                auto inicio =
                    high_resolution_clock::now();

                Resultado resultado =
                    ejecutarParalelo(
                        p,
                        politica
                    );

                auto fin =
                    high_resolution_clock::now();

                double tiempo =
                    duration<double>(
                        fin - inicio
                    ).count();

                tiempoTotal += tiempo;

                if (!iguales(
                        baseline,
                        resultado
                    )) {
                    correcto = false;
                }

                cout
                    << "  Corrida "
                    << repeticion
                    << ": "
                    << tiempo
                    << " s\n";
            }

            double Tp =
                tiempoTotal / REPETICIONES;

            double speedup =
                T1 / Tp;

            double eficiencia =
                speedup / p;

            cout
                << "\n  Tp promedio = "
                << Tp
                << " s\n";

            cout
                << "  Speedup      = "
                << speedup
                << "x\n";

            cout
                << "  Eficiencia   = "
                << eficiencia * 100.0
                << " %\n";

            cout
                << "  Correctitud  = "
                << (correcto ? "OK" : "ERROR")
                << "\n";

            csv
                << nombrePolitica(politica)
                << ","
                << p
                << ","
                << Tp
                << ","
                << speedup
                << ","
                << eficiencia * 100.0
                << ","
                << (correcto ? "SI" : "NO")
                << "\n";
        }
    }

    csv.close();

    cout
        << "\n============================================================\n";

    cout
        << "Experimentos terminados.\n";

    cout
        << "Archivo generado: resultados.csv\n";

    cout
        << "============================================================\n";

    return 0;
}