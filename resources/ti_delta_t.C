#include <TPaveStats.h>
#include <TStyle.h>

void ti_delta_t() {
  // Turn on stats: 1111 displays Name, Entries, Mean, and RMS.
  gStyle->SetOptStat(1111);

  // TI-decoded TDC Delta-T Histogram Script
  // Reads the 'ti' tree produced by TIDecode.
  // Reference channel = tdc_refChannel (lowest-numbered channel that fired).
  // Plots delta-t distributions for all channels in tdc_hit_channels
  // relative to that reference.

  TTree *t = (TTree *)gDirectory->Get("ti");
  if (!t) {
    std::cout
        << "Tree 'ti' not found. Make sure you selected a TI-decoded ROOT file."
        << std::endl;
    return;
  }

  // ===== Configuration =====
  // Channels to plot (relative to the reference channel stored in
  // tdc_refChannel). These are the *other* channels recorded in
  // tdc_hit_channels.
  std::vector<int> channels_to_plot = {1, 2, 3};
  std::vector<int> colors = {kBlue,   kRed,  kGreen + 2, kMagenta,
                             kOrange, kCyan, kBlack};
  // =========================

  TCanvas *c =
      new TCanvas("c_ti_tdc", "TI TDC #Delta t Distributions", 800, 600);
  c->cd();

  THStack *hs = new THStack(
      "hs_ti",
      "TI TDC #Delta t (ref = first hit channel);#Delta t (ns);Counts");

  // Moved the legend to the top left so it doesn't overlap with stat boxes on
  // the right
  TLegend *leg = new TLegend(0.15, 0.70, 0.35, 0.88);
  leg->SetBorderSize(0);

  // Vector to store pointers to our histograms so we can easily access them
  // later

  for (size_t i = 0; i < channels_to_plot.size(); ++i) {
    int ch = channels_to_plot[i];
    int color = colors[i % colors.size()];

    TString hName = TString::Format("h_ti_dt%d", ch);
    TH1F *h = new TH1F(hName, hName, 100, 900, 1000);
    h->SetLineColor(color);
    h->SetLineWidth(2);

    // tdc_hit_channels is a std::vector<Int_t> branch.
    // Use the index trick: draw tdc_delta_t for entries where the
    // corresponding element of tdc_hit_channels equals ch.
    TString drawCmd = TString::Format("tdc_delta_t >> %s", hName.Data());
    TString cutCmd = TString::Format("tdc_hit_channels == %d", ch);
    t->Draw(drawCmd, cutCmd, "goff");

    hs->Add(h);
    leg->AddEntry(h, TString::Format("#Delta t (ref - ch%d)", ch), "l");
  }

  // Draw the stack first to set up the axes
  hs->Draw("nostack hist");
  c->Update(); // Crucial: forces ROOT to physically draw the canvas

  // Get the list of histograms directly from the THStack to avoid std::vector
  // Cling errors
  TList *hList = hs->GetHists();
  if (hList) {
    for (int i = 0; i < hList->GetSize(); ++i) {
      TH1F *h = (TH1F *)hList->At(i);

      // "sames" tells ROOT to draw the stat box without overriding the stack
      h->Draw("sames hist");
      c->Update(); // Crucial: forces generation of the TPaveStats object

      TPaveStats *stats = (TPaveStats *)h->FindObject("stats");
      if (stats) {
        // Calculate new Y coordinates to stack the boxes vertically
        double height = 0.15;
        double y2 = 0.90 - (i * height);
        double y1 = y2 - height;

        stats->SetY1NDC(y1);
        stats->SetY2NDC(y2);

        // Match the stat box text color to the histogram line color
        stats->SetTextColor(h->GetLineColor());
      }
    }
  }

  leg->Draw();
  c->Update();
  // c->SaveAs("~/.../ti_scope_wavegen.png");
}