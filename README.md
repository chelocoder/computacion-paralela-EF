# Evaluación Final - Computación Paralela y Distribuida

Proyecto desarrollado para analizar el procesamiento masivo de datos climáticos utilizando programación paralela.

## Tecnologías utilizadas

- C++
- OpenMP
- Microsoft MPI

## Descripción

Se procesan 100 000 000 de mediciones climáticas sintéticas para comparar una ejecución secuencial con versiones paralelas.

Se evaluaron:

- tiempos de ejecución;
- speedup;
- eficiencia;
- políticas de scheduling en OpenMP;
- validación mediante MPI.

## Archivos principales

- `main.cpp`: implementación secuencial y paralela con OpenMP.
- `mpi_clima.cpp`: implementación utilizando MPI.
- `resultados.csv`: resultados de las pruebas con OpenMP.

## Compilación OpenMP

```bash
cl /EHsc /O2 /openmp main.cpp
