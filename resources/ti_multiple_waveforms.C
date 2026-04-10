void ti_multiple_waveforms() {
  // Draw multiple V1720 waveforms from a TI-decoded ROOT file (tree: "ti")
  // X-axis: sample index range 0-500
  // GUI: open a _ti_decoded.root file in the Analysis tab, then Run Script.

  TTree *t = (TTree *)gDirectory->Get("ti");
  if (!t) {
    std::cout
        << "Tree 'ti' not found. Make sure you selected a TI-decoded ROOT file."
        << std::endl;
    return;
  }

  // ===== Configuration =====
  int startEvent = 0; // first event to draw
  int nEvents = 10;   // number of waveforms
  // =========================

  // ===== Calibration =====
  int pedStart = 400; // sample index range for pedestal (within 0-500 window)
  int pedEnd = 480;
  int ADCdynamicRange = 2000; // mV
  int ADCbits = 12;
  double LSB = ADCdynamicRange / (pow(2, ADCbits) - 1);
  // =======================

  std::vector<int> eventPeds;
  std::vector<int> minValues;
  std::vector<double> calibratedMinValues;

  Long64_t totalEntries = t->GetEntries();
  if (startEvent >= totalEntries) {
    std::cout << "Start event exceeds total entries!" << std::endl;
    return;
  }
  if (startEvent + nEvents > totalEntries)
    nEvents = totalEntries - startEvent;

  std::vector<Float_t> *samples = nullptr;
  t->SetBranchAddress("ch0_samples", &samples);

  TCanvas *c =
      new TCanvas("c_ti_multi_wave", "TI – 10 Waveforms (ch0)", 1400, 800);
  c->Divide(5, 2);

  for (int ev = 0; ev < nEvents; ev++) {
    t->GetEntry(startEvent + ev);
    if (!samples || samples->empty())
      continue;

    int n = samples->size();

    // Clamp to 0-500 display window
    int displayEnd = std::min(n, 500);

    std::vector<double> x(displayEnd), y(displayEnd);
    for (int i = 0; i < displayEnd; i++) {
      x[i] = i;
      y[i] = samples->at(i);
    }

    // Pedestal (within the displayed window)
    int pedEndClamped = std::min(pedEnd, displayEnd);
    int pedStartClamped = std::min(pedStart, pedEndClamped);
    int pedestal = 0;
    for (int i = pedStartClamped; i < pedEndClamped; i++)
      pedestal += (int)y[i];
    if (pedEndClamped > pedStartClamped)
      pedestal /= (pedEndClamped - pedStartClamped);
    eventPeds.push_back(pedestal);

    int minVal = (int)*std::min_element(y.begin(), y.end());
    minValues.push_back(minVal);

    c->cd(ev + 1);
    TGraph *gr = new TGraph(displayEnd, &x[0], &y[0]);
    gr->SetTitle(Form("Event %d;Sample Number;ADC Value", startEvent + ev));
    gr->SetLineWidth(1);
    gr->GetXaxis()->SetRangeUser(0, 500);
    gr->Draw("AL");
  }
  c->Update();
  // c->SaveAs("~/MyFiles/daqmainFiles/tiData/plots/ti_scope_wavegen.png");

  // Average pedestal & calibrated minimum
  int avgPedestal = 0;
  if (!eventPeds.empty())
    avgPedestal = std::accumulate(eventPeds.begin(), eventPeds.end(), 0) /
                  (int)eventPeds.size();
  for (int i = 0; i < (int)minValues.size(); i++)
    calibratedMinValues.push_back((minValues[i] - avgPedestal) * LSB);

  // Summary printout
  std::cout << "\nEvent Pedestals:" << std::endl;
  for (int i = 0; i < (int)eventPeds.size(); i++)
    std::cout << "Event " << startEvent + i << ": " << eventPeds[i]
              << std::endl;
  std::cout << "----------------------------------------" << std::endl;
  std::cout << "Average Pedestal: " << avgPedestal << std::endl;
  std::cout << "----------------------------------------" << std::endl;

  std::cout << "\nMinimum Values:" << std::endl;
  for (int i = 0; i < (int)minValues.size(); i++)
    std::cout << "Event " << startEvent + i << ": " << minValues[i]
              << std::endl;

  std::cout << "\nCalibrated Minimum Values:" << std::endl;
  for (int i = 0; i < (int)calibratedMinValues.size(); i++)
    std::cout << "Event " << startEvent + i << ": " << calibratedMinValues[i]
              << " mV" << std::endl;
}
