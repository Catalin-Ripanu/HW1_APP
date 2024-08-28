#include <bits/stdc++.h>
#include <sys/time.h>
#include <mpi.h>
using namespace std;
#define MASTER 0

vector<pair<double, double>> centroids;
vector<pair<double, double>> points;
vector<int> output;

const int iters = 100;

double dist(pair<double, double> pa1, pair<double, double> pa2)
{
    return (pa1.first - pa2.first) * (pa1.first - pa2.first) +
           (pa1.second - pa2.second) * (pa1.second - pa2.second);
}

int find_nearest_cluster(vector<pair<double, double>> &centroids, vector<pair<double, double>> &points, int K, int j)
{
    int cl = -1;
    int min_dist = INT_MAX;

    for (int l = 0; l < K; l++)
        if (min_dist > dist(centroids[l], points[j]))
        {
            min_dist = dist(centroids[l], points[j]);
            cl = l;
        }
    return cl;
}

void master_process_data(double *meanx, double *meany, double *counts, double *aux_data_vec,
                         vector<pair<double, double>> &centroids, double *send_data_vec,
                         vector<pair<double, double>> &points, int K, int chunk_size, int nrprocs)
{
    for (int i = 0; i < iters; i++)
    {
        for (int j = 0; j < K; j++)
        {
            meanx[j] = 0;
            meany[j] = 0;
            counts[j] = 0;
        }
        for (int j = 0; j < chunk_size; j++)
        {
            int cl = find_nearest_cluster(centroids, points, K, j);

            if (cl != -1)
            {
                meanx[cl] += points[j].first;
                meany[cl] += points[j].second;
                counts[cl]++;
            }
        }

        for (int j = 1; j < nrprocs; j++)
        {
            MPI_Recv(send_data_vec, 3 * K, MPI_DOUBLE, j, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int l = 0; l < K; l++)
                if (send_data_vec[l * 3 + 2] != 0)
                {
                    meanx[l] += send_data_vec[l * 3];
                    meany[l] += send_data_vec[l * 3 + 1];
                    counts[l] += send_data_vec[l * 3 + 2];
                }
        }

        for (int j = 0; j < K; j++)
            if (counts[j] != 0)
            {
                centroids[j] = make_pair(meanx[j] / counts[j], meany[j] / counts[j]);
            }

        for (int j = 0; j < K; j++)
        {
            aux_data_vec[2 * j] = centroids[j].first;
            aux_data_vec[2 * j + 1] = centroids[j].second;
        }

        for (int j = 1; j < nrprocs; j++)
        {
            MPI_Send(aux_data_vec, 2 * K, MPI_DOUBLE, j, 0, MPI_COMM_WORLD);
        }
    }

    for (int j = 0; j < K; j++)
    {
        aux_data_vec[2 * j] = centroids[j].first;
        aux_data_vec[2 * j + 1] = centroids[j].second;
    }

    for (int j = 1; j < nrprocs; j++)
    {
        MPI_Send(aux_data_vec, 2 * K, MPI_DOUBLE, j, 0, MPI_COMM_WORLD);
    }
}

void master_write_output(double *aux_data_vec, int chunk_size, int N, int K, int nrprocs,
                         vector<pair<double, double>> &centroids, vector<pair<double, double>> &points, ofstream &fout)
{
    for (int j = 0; j < chunk_size; j++)
    {
        int cl = find_nearest_cluster(centroids, points, K, j);

        fout << cl << '\n';
    }

    for (int j = 1; j < nrprocs - 1; j++)
    {
        MPI_Recv(aux_data_vec, chunk_size, MPI_DOUBLE, j, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < chunk_size; i++)
            fout << aux_data_vec[i] << '\n';
    }

    MPI_Recv(aux_data_vec, N - (nrprocs - 1) * chunk_size, MPI_DOUBLE, nrprocs - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    for (int i = 0; i < N - (nrprocs - 1) * chunk_size; i++)
        fout << aux_data_vec[i] << '\n';

    free(aux_data_vec);
    fout.close();
}

void non_master_process_data(double *meanx, double *meany, double *counts, double *aux_data_vec,
                             vector<pair<double, double>> &centroids, double *send_data_vec,
                             vector<pair<double, double>> &points, int K, int chunk_size, int recv_size)
{
    for (int i = 0; i < iters; i++)
    {
        for (int j = 0; j < K; j++)
        {
            meanx[j] = 0;
            meany[j] = 0;
            counts[j] = 0;
        }
        for (int j = 0; j < recv_size; j++)
        {
            int cl = find_nearest_cluster(centroids, points, K, j);

            if (cl != -1)
            {
                meanx[cl] += points[j].first;
                meany[cl] += points[j].second;
                counts[cl]++;
            }
        }

        for (int j = 0; j < K; j++)
        {
            send_data_vec[3 * j + 2] = counts[j];
            send_data_vec[3 * j] = meanx[j];
            send_data_vec[3 * j + 1] = meany[j];
        }

        MPI_Send(send_data_vec, 3 * K, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

        MPI_Recv(aux_data_vec, 2 * K, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        centroids.clear();

        for (int j = 0; j < K; j++)
        {
            centroids.push_back(make_pair(aux_data_vec[2 * j], aux_data_vec[2 * j + 1]));
        }
    }

    MPI_Recv(aux_data_vec, 2 * K, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    centroids.clear();

    for (int j = 0; j < K; j++)
    {
        centroids.push_back(make_pair(aux_data_vec[2 * j], aux_data_vec[2 * j + 1]));
    }

    for (int j = 0; j < recv_size; j++)
    {
        int cl = find_nearest_cluster(centroids, points, K, j);

        aux_data_vec[j] = cl;
    }

    MPI_Send(aux_data_vec, recv_size, MPI_DOUBLE, MASTER, 0, MPI_COMM_WORLD);
    free(aux_data_vec);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, nrprocs;

    int N, K;

    MPI_Comm_size(MPI_COMM_WORLD, &nrprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == MASTER)
    {
        ifstream fin(argv[1]);
        ofstream fout("output_MPI.txt");

        fin >> K;
        fin >> N;

        int cl;
        double min_dist;
        double *meanx, *meany, *counts, *send_data_vec;

        meanx = (double *)malloc(K * sizeof(double));
        meany = (double *)malloc(K * sizeof(double));
        counts = (double *)malloc(K * sizeof(double));
        send_data_vec = (double *)malloc(3 * K * sizeof(double));

        for (int i = 0; i < N; i++)
        {
            double x_comp, y_comp;
            fin >> x_comp >> y_comp;
            points.push_back(make_pair(x_comp, y_comp));
        }

        srand(time(NULL));

        for (int i = 0; i < K; i++)
            centroids.push_back(points[rand() % N]);

        fin.close();

        MPI_Bcast(&N, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
        MPI_Bcast(&K, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

        int chunk_size = N / nrprocs;

        int size = chunk_size;
        if (size < K)
            size = K;

        if (size < N - (nrprocs - 1) * chunk_size)
        {
            size = N - (nrprocs - 1) * chunk_size;
        }

        double *aux_data_vec = (double *)malloc(size * nrprocs * sizeof(double));

        for (int i = 1; i < nrprocs - 1; i++)
        {
            int l = 0;
            for (int j = i * chunk_size; j < (i + 1) * chunk_size; j++)
            {
                aux_data_vec[2 * l] = points[j].first;
                aux_data_vec[2 * l + 1] = points[j].second;
                l++;
            }
            MPI_Send(aux_data_vec, 2 * chunk_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }

        int l = 0;
        for (int i = (nrprocs - 1) * chunk_size; i < N; i++)
        {
            aux_data_vec[2 * l] = points[i].first;
            aux_data_vec[2 * l + 1] = points[i].second;
            l++;
        }

        if (nrprocs > 1)
            MPI_Send(aux_data_vec, 2 * (N - (nrprocs - 1) * chunk_size), MPI_DOUBLE, nrprocs - 1, 0, MPI_COMM_WORLD);

        for (int i = 0; i < K; i++)
        {
            aux_data_vec[2 * i] = centroids[i].first;
            aux_data_vec[2 * i + 1] = centroids[i].second;
        }

        for (int i = 1; i < nrprocs; i++)
        {
            MPI_Send(aux_data_vec, 2 * K, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }

        master_process_data(meanx, meany, counts, aux_data_vec, centroids, send_data_vec, points, K, chunk_size, nrprocs);

        master_write_output(aux_data_vec, chunk_size, N, K, nrprocs, centroids, points, fout);
    }
    else
    {
        MPI_Bcast(&N, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
        MPI_Bcast(&K, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

        int cl;
        double min_dist;
        double *meanx, *meany, *counts, *send_data_vec;

        meanx = (double *)malloc(K * sizeof(double));
        meany = (double *)malloc(K * sizeof(double));
        counts = (double *)malloc(K * sizeof(double));
        send_data_vec = (double *)malloc(3 * K * sizeof(double));

        int chunk_size = N / nrprocs;

        int size = chunk_size;
        if (size < K)
            size = K;

        if (rank == nrprocs - 1)
        {
            size = N - (nrprocs - 1) * chunk_size;
        }

        int recv_size = chunk_size;

        if (rank == nrprocs - 1)
            recv_size = N - (nrprocs - 1) * chunk_size;

        double *aux_data_vec = (double *)malloc(size * nrprocs * sizeof(double));

        MPI_Recv(aux_data_vec, 2 * recv_size, MPI_DOUBLE, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < recv_size; i++)
        {
            points.push_back(make_pair(aux_data_vec[2 * i], aux_data_vec[2 * i + 1]));
        }

        MPI_Recv(aux_data_vec, 2 * K, MPI_DOUBLE, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < K; i++)
        {
            centroids.push_back(make_pair(aux_data_vec[2 * i], aux_data_vec[2 * i + 1]));
        }

        non_master_process_data(meanx, meany, counts, aux_data_vec, centroids, send_data_vec, points, K, chunk_size, recv_size);
    }

    MPI_Finalize();
    return 0;
}