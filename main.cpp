#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;

const int dr[] = {0,1,1,1,0,-1,-1,-1};
const int dc[] = {1,1,0,-1,-1,-1,0,1};

using table = array<array<int, 14>, 8>;

struct Checker {
  array<bool, 10001> chk;

  void dfs(int r, int c, int d, int n, const table &a) {
    chk[n] = 1;
    if (d == 4) return;
    for (int k=0;k<8;++k) {
      int nr = r + dr[k];
      int nc = c + dc[k];
      if (nr < 0 || nr >= 8 || nc < 0 || nc >= 14) continue;
      dfs(nr, nc, d+1, n*10+a[nr][nc], a);
    }
  }

  int eval(const table &a) {
    fill(chk.begin(), chk.end(), 0);
    for (int i=0;i<8;++i) 
      for (int j=0;j<14;++j) 
        dfs(i, j, 1, a[i][j], a);
      
    int score = find(chk.begin(), chk.end(), 0) - chk.begin() - 1;
    return score;
  }
};

Checker checker;
table a;
mt19937 mt(time(0));

uniform_real_distribution<> uniform(0, 1);
uniform_int_distribution<> randd(0, 9);
uniform_int_distribution<> randr(0, 7);
uniform_int_distribution<> randc(0, 13);
uniform_int_distribution<> rand4(0, 4);

void mutate(table &t) {
  int r = randr(mt);
  int c = randc(mt);
  int x = randd(mt);
  t[r][c] = x;
}

double reg_factor=1000;
double reg(double x) {
  double ret = 0;
  double ratio = 1;
  while (x > reg_factor) {
    ret += reg_factor / ratio;
    ratio*=2;
    x -= reg_factor;
  }
  return ret + x / ratio;
}

struct SA {
  table best;
  int64_t best_gen;
  table cur, nxt;

  int cnt = 0;
  double stationary = 0;
  int mutated=0;

  int run(double temperature_init, double mn, bool hard=0, int period=1e6, double decay=0.99999) {
    int e1, e2;
    int64_t gen=0;
    double temperature = temperature_init;


    e1 = checker.eval(cur);
    best = cur;
    int best_score=e1;
    double best_reg = reg(e1);
    double avg_mean = e1;
    double stationary = 0;

    while (1) {
      nxt = cur;
      mutate(nxt);

      e2 = checker.eval(nxt);

      double re1 = reg(e1);
      double re2 = reg(e2);

      avg_mean = avg_mean * 0.999 + re2 * 0.001;

      if (avg_mean * 0.7 < re2 && re2 < avg_mean * 1.3) {
        stationary += 0.01;
      }
      else stationary *= 0.99;

      double prob = exp((re2-re1)/(1+stationary+temperature));
      if (uniform(mt) <= prob) {
        swap(cur, nxt);
        e1 = e2;
        mutated++;
      }

      temperature *= decay;
      temperature = max(mn, temperature);

      if (e2 > best_score) {
        best_score = e2;
        best_reg = re2;
        best_gen = gen;
        best = cur;
      }
      
      if (hard && gen > best_gen + period) {
        best_gen = gen;
        if (hard) {
          cur = best;
          e2 = best_score;
        }
        temperature = temperature_init;
      }


      // ==============================
      //         PRINT INFO
      // ==============================
      if (gen++ % 1000 == 0) {
        // sleep(1);
        // cout << string(10, '=') << " Best p1e6 " <<  best_p1e6_score << " " << reg(best_p1e6_score) << " " << string(10, '=') << '\n';
        cout << string(10, '=') << " Best " <<  best_score << " " << best_reg << " " << string(10, '=') << '\n';
        for (auto &row : best) {
          for (int x : row) cout << x << ' ';
          cout << '\n';
        }
        cout << string(10, '=') << " Current " << e1 << " " << re1 << " " << string(10, '=') << '\n';
        for (auto &row : cur) {
          for (int x : row) cout << x << ' ';
          cout << '\n';
        }
        cout << "Generation " << gen <<  ' ' << best_gen << '\n';
        cout << "temperature " << temperature
        << " stationary " << stationary
        << " mutated " << mutated << '\n';
        cout << "temperature_init " << temperature_init << ", " << "hard " << hard
          << " period " << period << " reg factor " << reg_factor << '\n';
        mutated=0;
      }
    }
  }
};

SA sa;

int main(int argc, char *argv[]) {
  // Load given table
  if (argc >= 2) {
    string s(argv[1]);
    ifstream fin(s);
    cout << "File is " << s << "\n";
    if (!fin) { cerr << "no such file. "; }
    else {
      for (int i=0;i<8;++i) for (int j=0;j<14;++j) fin >> sa.cur[i][j];
      cout << "Set!\n";
    }
  }

  for (auto &row : sa.cur) {
    for (int x : row) cout << x << ' ';
    cout << '\n';
  }
  cout << checker.eval(sa.cur) << '\n';

  double init;
  double mn;
  bool hard;
  int period;
  double decay=0.99999;
  cout << "Specify reg factor (~1000) >";
  cin >> reg_factor;
  cout << "Specify temperature_init (~10) >";
  cin >> init;
  cout << "Specify minimum temperature (~0) >";
  cin >> mn;
  cout << "Specify whether hard reset to best when deadlock (0/1) >";
  cin >> hard;
  if (hard) { 
    cout << "Specify reset period >";
    cin >> period;
  }

  cout << fixed << setprecision(3);
  sa.run(init, mn, hard, period, decay);
}
