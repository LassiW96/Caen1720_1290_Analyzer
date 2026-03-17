void single_waveform() {
   // Example Script to draw a single waveform of Channel 0 using TGraph
   // For now, it will draw the first event, change as required.
   // The GUI automatically opens the selected ROOT file for you.

   TTree *t = (TTree*)gDirectory->Get("t1");
   if (!t) {
       std::cout << "Tree 't1' not found. Make sure you selected a file." << std::endl;
       return;
   }

   // Pointer to waveform vector
   std::vector<int> *samples = nullptr;
   t->SetBranchAddress("ch0_samples", &samples);

   // Load first event (you can change event number if needed)
   t->GetEntry(0);

   if (!samples) {
       std::cout << "No waveform data found." << std::endl;
       return;
   }

   int n = samples->size();

   // Create arrays for TGraph
   std::vector<double> x(n), y(n);
   for (int i = 0; i < n; i++) {
       x[i] = i;                 // Sample index (time bin)
       y[i] = samples->at(i);    // ADC value
   }

   TCanvas *c = new TCanvas("c_wave", "Channel 0 Waveform", 800, 600);
   c->cd();

   TGraph *gr = new TGraph(n, &x[0], &y[0]);
   gr->SetTitle("Channel 0 Waveform;Sample Number;ADC Value");
   gr->SetLineWidth(2);
   gr->Draw("AL");  // A = axes, L = line

   c->Update();
}
