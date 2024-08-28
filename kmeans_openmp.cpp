#include <bits/stdc++.h>
#include <sys/time.h>

#include <omp.h>

using namespace std;

ifstream fin("data/test1.in");
ofstream fout("data/out1.out");

vector<pair<double, double>> centroids;

vector<pair<double, double>> points;
vector<int> points_to_clusters;

vector<int> final_result;

int iters = 100;

double euclidean_distance(pair<double, double> pa1, pair<double, double> pa2) {
    return (pa1.first - pa2.first) * (pa1.first - pa2.first) +
        (pa1.second - pa2.second) * (pa1.second - pa2.second);
}

int find_nearest_cluster(pair<double, double> point) {
    int cluster_no = -1;
    int minimum_distance = INT_MAX;

    for (int i = 0; i < centroids.size(); i++) {
        double current_distance = euclidean_distance(point, centroids[i]);
        if (minimum_distance > current_distance) {
            minimum_distance = current_distance;
            cluster_no = i;
        }
    }

    return cluster_no;
}

void compute(int N, int K) {
    int i, j, cl, k, l;
    double meanx = 0, meany = 0;

    for (i = 0; i < iters; i++) {
        #pragma omp parallel for private(j) shared(points, points_to_clusters) schedule(dynamic, 32)
        for (j = 0; j < N; j++) {
            int cluster_no = find_nearest_cluster(points[j]);

            if (cluster_no != -1) {
                // Replaced clusters[cluster_no].push_back(points[j]);
                // to avoid using a lock on clusters (which was
                // highly inefficient, incurring an idle time ~2x greater
                // than the computation time)
                points_to_clusters[j] = cluster_no;
            }
        }

        #pragma omp parallel for private(j) schedule(dynamic)
        for (j = 0; j < K; j++) {
            int cluster_size = 0;

            #pragma omp parallel for reduction(+:meanx, meany, cluster_size) private(l) schedule(dynamic, 32)
            for (l = 0; l < N; l++) {
                if (points_to_clusters[l] == j) {
                    meanx += points[l].first;
                    meany += points[l].second;

                    cluster_size += 1;
                }
            }

            // Update the j-th centroid based on the average on both
            // dimensions for all the point located in its cluster
            if (cluster_size != 0) {
                centroids[j].first = meanx / cluster_size;
                centroids[j].second = meany / cluster_size;
            }

            meanx = 0;
            meany = 0;
        }
    }

    #pragma opm parallel for private(i)
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

    points_to_clusters.resize(N);
    final_result.resize(N);

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

    return 0;
}