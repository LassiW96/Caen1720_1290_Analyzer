void multiple_waveforms_calibrated() {
  // Draw multiple waveforms from ch0_samples
  // GUI automatically opens selected ROOT file

  TTree *t = (TTree *)gDirectory->Get("t1");
  if (!t) {
    std::cout << "Tree 't1' not found. Make sure you selected a file."
              << std::endl;
    return;
  }

  // ===== No of events to draw =====
  int startEvent = 0; // First event to draw
  int nEvents = 10;   // Number of waveforms to draw
  // ================================

  // ===== Calibration and pedestal =====
  int pedStart = 3000;
  int pedEnd = 3020;
  std::vector<int> eventPeds;

  int ADCdynamicRange = 2000; // mV
  int ADCbits = 12;           // 12-bit ADC
  double LSB = ADCdynamicRange / (pow(2, ADCbits) - 1);
  // ====================================

  // Vector to store minimum values
  std::vector<int> minValues;
  std::vector<double> calibratedMinValues;

  Long64_t totalEntries = t->GetEntries();

  if (startEvent >= totalEntries) {
    std::cout << "Start event exceeds total entries!" << std::endl;
    return;
  }

  if (startEvent + nEvents > totalEntries) {
    nEvents = totalEntries - startEvent;
  }

  std::vector<int> *samples = nullptr;
  t->SetBranchAddress("ch0_samples", &samples);

  // Create canvas and divide (5x2 layout for 10 plots)
  TCanvas *c = new TCanvas("c_multi_wave", "10 Waveforms", 1400, 800);
  c->Divide(5, 2);

  for (int ev = 0; ev < nEvents; ev++) {
    int pedestal = 0;
    t->GetEntry(startEvent + ev);

    if (!samples)
      continue;

    int n = samples->size();
    if (n == 0)
      continue;

    std::vector<double> x(n), y(n);

    for (int i = 0; i < n; i++) {
      x[i] = i;
      y[i] = samples->at(i);
    }
    minValues.push_back(*std::min_element(y.begin(), y.end()));

    // Pedestal calculation
    for (int i = pedStart; i < pedEnd; i++) {
      pedestal += y[i];
    }
    pedestal /= (pedEnd - pedStart);
    eventPeds.push_back(pedestal);

    c->cd(ev + 1);

    TGraph *gr = new TGraph(n, &x[0], &y[0]);
    gr->SetTitle(Form("Event %d;Sample Number;ADC Value", startEvent + ev));
    gr->SetLineWidth(1);
    gr->GetXaxis()->SetRangeUser(2900, 3080);
    gr->Draw("AL");
  }
  c->Update();

  // Average pedestal
  int avgPedestal =
      std::accumulate(eventPeds.begin(), eventPeds.end(), 0) / eventPeds.size();

  // Calibrated min values
  for (int i = 0; i < minValues.size(); i++) {
    calibratedMinValues.push_back((minValues[i] - avgPedestal) * LSB);
  }

  // Print event pedestals
  std::cout << "\nEvent Pedestals:" << std::endl;
  for (int i = 0; i < eventPeds.size(); i++) {
    std::cout << "Event " << startEvent + i << ": " << eventPeds[i]
              << std::endl;
  }

  std::cout << "----------------------------------------" << std::endl;
  std::cout << "Average Pedestal: " << avgPedestal << std::endl;
  std::cout << "----------------------------------------" << std::endl;

  // Print minimum values
  std::cout << "\nMinimum Values:" << std::endl;
  for (int i = 0; i < minValues.size(); i++) {
    std::cout << "Event " << startEvent + i << ": " << minValues[i]
              << std::endl;
  }

  std::cout << "\nCalibrated Minimum Values:" << std::endl;
  for (int i = 0; i < calibratedMinValues.size(); i++) {
    std::cout << "Event " << startEvent + i << ": " << calibratedMinValues[i]
              << " mV" << std::endl;
  }
}
