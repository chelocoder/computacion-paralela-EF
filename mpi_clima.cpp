#include <iostream>
#include <iomanip>
#include <chrono>
#include <mpi.h>

using namespace std;
using namespace chrono;

const long long N = 100000000LL;

struct Resultado {
    long long sumaTemperatura;
    long long sumaPrecipitacion;
    long long sumaHumedad;
    long long sumaCaudal;
    long long alertas;
};

inline unsigned int hashDato(unsigned long long x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;

    return static_cast<unsigned int>(x);
}

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

    temperatura = 100 + (h1 % 300);
    precipitacion = h2 % 2000;
    humedad = 30 + (h3 % 70);
    caudal = 50 + ((h1 ^ h2) % 1000);

    alerta =
        (precipitacion > 1500 &&
         humedad > 85 &&
         caudal > 800)
        ? 1
        : 0;
}

int main(int argc, char* argv[]) {

    MPI_Init(&argc, &argv);

    int rank;
    int size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    long long bloque = N / size;

    long long inicio = rank * bloque;

    long long fin =
        (rank == size - 1)
        ? N
        : inicio + bloque;

    long long sumaTemperaturaLocal = 0;
    long long sumaPrecipitacionLocal = 0;
    long long sumaHumedadLocal = 0;
    long long sumaCaudalLocal = 0;
    long long alertasLocal = 0;

    MPI_Barrier(MPI_COMM_WORLD);

    double tiempoInicio = MPI_Wtime();

    for (long long i = inicio; i < fin; ++i) {

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

        sumaTemperaturaLocal += temperatura;
        sumaPrecipitacionLocal += precipitacion;
        sumaHumedadLocal += humedad;
        sumaCaudalLocal += caudal;
        alertasLocal += alerta;
    }

    long long sumaTemperaturaGlobal = 0;
    long long sumaPrecipitacionGlobal = 0;
    long long sumaHumedadGlobal = 0;
    long long sumaCaudalGlobal = 0;
    long long alertasGlobal = 0;

    MPI_Reduce(
        &sumaTemperaturaLocal,
        &sumaTemperaturaGlobal,
        1,
        MPI_LONG_LONG,
        MPI_SUM,
        0,
        MPI_COMM_WORLD
    );

    MPI_Reduce(
        &sumaPrecipitacionLocal,
        &sumaPrecipitacionGlobal,
        1,
        MPI_LONG_LONG,
        MPI_SUM,
        0,
        MPI_COMM_WORLD
    );

    MPI_Reduce(
        &sumaHumedadLocal,
        &sumaHumedadGlobal,
        1,
        MPI_LONG_LONG,
        MPI_SUM,
        0,
        MPI_COMM_WORLD
    );

    MPI_Reduce(
        &sumaCaudalLocal,
        &sumaCaudalGlobal,
        1,
        MPI_LONG_LONG,
        MPI_SUM,
        0,
        MPI_COMM_WORLD
    );

    MPI_Reduce(
        &alertasLocal,
        &alertasGlobal,
        1,
        MPI_LONG_LONG,
        MPI_SUM,
        0,
        MPI_COMM_WORLD
    );

    double tiempoFin = MPI_Wtime();

    double tiempoLocal =
        tiempoFin - tiempoInicio;

    double tiempoGlobal = 0.0;

    MPI_Reduce(
        &tiempoLocal,
        &tiempoGlobal,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        MPI_COMM_WORLD
    );

    if (rank == 0) {

        cout << fixed << setprecision(6);

        cout << "============================================================\n";
        cout << " VALIDACION MPI - DATOS CLIMATICOS DEL PERU\n";
        cout << "============================================================\n\n";

        cout << "Procesos MPI          : "
             << size << "\n";

        cout << "Mediciones procesadas : "
             << N << "\n";

        cout << "Tiempo global MPI     : "
             << tiempoGlobal
             << " s\n\n";

        cout << "Temperatura promedio  : "
             << (sumaTemperaturaGlobal / (double)N) / 10.0
             << " C\n";

        cout << "Precipitacion promedio: "
             << (sumaPrecipitacionGlobal / (double)N) / 10.0
             << " mm\n";

        cout << "Humedad promedio      : "
             << sumaHumedadGlobal / (double)N
             << " %\n";

        cout << "Caudal promedio       : "
             << sumaCaudalGlobal / (double)N
             << " m3/s\n";

        cout << "Alertas detectadas    : "
             << alertasGlobal
             << "\n";

        cout << "\n============================================================\n";
    }

    MPI_Finalize();

    return 0;
}