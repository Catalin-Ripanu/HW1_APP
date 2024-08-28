#include <bits/stdc++.h>
#include <sys/time.h>
#include <pthread.h>

using namespace std;

#define NUM_THREADS 32

ifstream fin("data/test1.in");
ofstream fout("data/out1_pthreads.out");

vector<vector<pair<double, double>>> clusters;
vector<pthread_mutex_t> clusters_locks;

vector<pair<double, double>> centroids;

vector<pair<double, double>> points;
vector<int> final_result;

int iters = 100;

double euclidean_distance(pair<double, double> pa1, pair<double, double> pa2) {
    return (pa1.first - pa2.first) * (pa1.first - pa2.first) +
        (pa1.second - pa2.second) * (pa1.second - pa2.second);
}

int find_nearest_cluster(pair<double, double> point) {
    int cluster_no = -1;
    int minimum_distance = INT_MAX;

    int i = 0;

    for (i = 0; i < centroids.size(); i++) {
        double current_distance = euclidean_distance(point, centroids[i]);

        if (minimum_distance > current_distance) {
            minimum_distance = current_distance;
            cluster_no = i;
        }
    }

    return cluster_no;
}

struct f_data {
    int thread_id;
    pair<int, int> interval;
};

void *f_assign_points_to_centroids(void *arg) {
    f_data *data = (f_data *) arg;

    for (int i = data->interval.first; i < data->interval.second; ++i) {
        int cluster_no = find_nearest_cluster(points[i]);

        if (cluster_no != -1) {
            pthread_mutex_lock(&clusters_locks[cluster_no]);
            clusters[cluster_no].push_back(points[i]);
            pthread_mutex_unlock(&clusters_locks[cluster_no]);
        }
    }

    return NULL;
}

void *f_update_centroids(void *arg) {
    f_data *data = (f_data *) arg;

    for (int j = data->interval.first; j < data->interval.second; ++j) {
        double meanx = 0;
        double meany = 0;

        for (int l = 0; l < clusters[j].size(); l++) {
            meanx += clusters[j][l].first;
            meany += clusters[j][l].second;
        }

        // Update the j-th centroid based on the average on both
        // dimensions for all the point located in its cluster
        if (clusters[j].size() != 0) {
            centroids[j].first = meanx / clusters[j].size();
            centroids[j].second = meany / clusters[j].size();
        }

        clusters[j].clear();
    }

    return NULL;
}

void compute(int N, int K) {
    int i, j, cl, k, l;
    double meanx, meany;
    double minimum_distance;

    for (i = 0; i < iters; i++) {
        pthread_t threads[NUM_THREADS];
        f_data threads_data[NUM_THREADS];

        int chunk_size = N / NUM_THREADS;

        for (k = 0; k < NUM_THREADS; ++k) {
            threads_data[k] = (f_data) {
                .thread_id = k,
                .interval = {k * chunk_size, (k + 1) * chunk_size},
            };

            if (k == NUM_THREADS - 1) {
                threads_data[k].interval = {threads_data[k].interval.first, N};
            }

            pthread_create(&threads[k], NULL, f_assign_points_to_centroids, &threads_data[k]);
        }

        for (int k = 0; k < NUM_THREADS; ++k) {
            pthread_join(threads[k], NULL);
        }

        chunk_size = K / NUM_THREADS;
        for (k = 0; k < NUM_THREADS; ++k) {
            threads_data[k] = (f_data) {
                .thread_id = k,
                .interval = {k * chunk_size, (k + 1) * chunk_size},
            };

            if (k == NUM_THREADS - 1) {
                threads_data[k].interval = {threads_data[k].interval.first, K};
            }

            pthread_create(&threads[k], NULL, f_update_centroids, &threads_data[k]);
        }

        for (int k = 0; k < NUM_THREADS; ++k) {
            pthread_join(threads[k], NULL);
        }
    }

    for (i = 0; i < N; i++) {
        final_result[i] = find_nearest_cluster(points[i]);
    }
}

int main() {
    double x, y;
    int N, K;
    fin >> K; fin >> N;

    srand(42);
    for (int i = 0; i < N; i++) {
        fin >> x >> y;
        points.push_back({x, y});
    }

    unordered_set<int> used_indices;
    for (int i = 0; i < K; i++) {
        int random_centroid;
        while (random_centroid = rand() % N, used_indices.count(random_centroid) != 0) {}

        used_indices.insert(random_centroid);
        centroids.push_back(points[random_centroid]);
    }

    clusters.resize(K);
    clusters_locks.resize(K);
    final_result.resize(N);

    // Initialize mutexes on clusters array
    for (int i = 0; i < K; ++i) {
        if (pthread_mutex_init(&clusters_locks[i], NULL) != 0) {
            exit(-1);
        }
    }

    auto start = std::chrono::high_resolution_clock::now();
    compute(N, K);
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    cout << "Serial KMeans took " << duration.count() / 1000.f << " seconds" << endl;

    for (int i = 0; i < N; i++) {
        fout << final_result[i] << '\n';
    }

    fout.close();
    fin.close();

    for (int i = 0; i < K; ++i) {
        pthread_mutex_destroy(&clusters_locks[i]);
    }

    return 0;
}