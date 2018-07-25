"""
A replacement module for the standard CosmoHammer.CosmoHammerSampler module.

The samplers in this module provide the ability to continue sampling if the sampling is discontinued for any reason.
Two samplers are provided -- one which works for emcee versions 3+, and one which works for the default v2. Note that
the output file structure looks quite different for these versions.
"""
from cosmoHammer import CosmoHammerSampler as CHS
import time
import numpy as np


class CosmoHammerSampler(CHS):
    def __init__(self, likelihoodComputationChain, continue_sampling=False, *args, **kwargs):
        self.continue_sampling = continue_sampling

        super().__init__(params=likelihoodComputationChain.params,
                         likelihoodComputationChain=likelihoodComputationChain,
                         *args, **kwargs)

        if not self.reuseBurnin:
            self.storageUtil.reset(self.nwalkers, self.paramCount)

        if not continue_sampling:
            self.storageUtil.reset(self.nwalkers, self.paramCount, burnin=False)

        if not self.storageUtil.burnin_initialized:
            self.storageUtil.reset(self.nwalkers, self.paramCount, burnin=True, samples=False)
            with self.storageUtil.burnin_storage.open() as f:
                print(list(f.keys()))
        if not self.storageUtil.samples_initialized:
            self.storageUtil.reset(self.nwalkers, self.paramCount, burnin=False, samples=True)
            ''
        if self.storageUtil.burnin_storage.iteration >= self.burninIterations:
            self.log("all burnin iterations already completed")
        if self.storageUtil.sample_storage.iteration >= self.sampleIterations:
            raise Exception("All Samples have already been completed. Try with continue_sampling=False.")

        if self.storageUtil.sample_storage.iteration > 0 and self.storageUtil.burnin_storage.iteration < self.burninIterations:
            self.log("resetting sample iterations because more burnin iterations requested.")
            self.storageUtil.reset(self.nwalkers, self.paramCount, samples=True)

    def startSampling(self):
        """
        Launches the sampling
        """
        try:
            if self.isMaster(): self.log(self.__str__())

            prob = None
            rstate = None
            datas = None
            pos = None
            if self.storageUtil.burnin_storage.iteration < self.burninIterations:
                if self.burninIterations:
                    if self.storageUtil.burnin_storage.iteration:
                        pos, prob, rstate, datas = self.loadBurnin()

                    if self.storageUtil.burnin_storage.iteration < self.burninIterations:
                        pos, prob, rstate, datas = self.startSampleBurnin(pos, prob, rstate, datas)
                else:
                    pos = self.createInitPos()
            else:
                if self.storageUtil.sample_storage.iteration:
                    pos, prob, rstate, datas = self.loadSamples()

                else:
                    pos = self.createInitPos()

            # Starting from the final position in the burn-in chain, start sampling.
            self.log("start sampling after burn in")
            start = time.time()
            self.sample(pos, prob, rstate, datas)
            end = time.time()
            self.log("sampling done! Took: " + str(round(end - start, 4)) + "s")

            # Print out the mean acceptance fraction. In general, acceptance_fraction
            # has an entry for each walker
            self.log("Mean acceptance fraction:" + str(round(np.mean(self._sampler.acceptance_fraction), 4)))
        finally:
            if self._sampler.pool is not None:
                try:
                    self._sampler.pool.close()
                except AttributeError:
                    pass
                try:
                    self.storageUtil.close()
                except AttributeError:
                    pass

    def loadBurnin(self):
        """
        loads the burn in form the file system
        """
        self.log("reusing previous burnin: %s iterations"%self.storageUtil.burnin_storage.iteration)
        return self.storageUtil.burnin_storage.get_last_sample()

    def loadSamples(self):
        """
        loads the samples form the file system
        """
        self.log("reusing previous samples: %s iterations"%self.storageUtil.sample_storage.iteration)
        pos, prob, rstate, data = self.storageUtil.sample_storage.get_last_sample()
        data = [{k:d[k] for k in d.dtype.names} for d in data]
        return pos, prob, rstate, data

    def startSampleBurnin(self, pos=None, prob=None, rstate=None, data=None):
        """
        Runs the sampler for the burn in
        """
        if self.storageUtil.burnin_storage.iteration:
            self.log("continue burn in")
        else:
            self.log("start burn in")
        start = time.time()

        if pos is None: pos = self.createInitPos()
        pos, prob, rstate, data = self.sampleBurnin(pos, prob, rstate, data)
        end = time.time()
        self.log("burn in sampling done! Took: " + str(round(end - start, 4)) + "s")
        self.log("Mean acceptance fraction for burn in:" + str(round(np.mean(self._sampler.acceptance_fraction), 4)))

        self.resetSampler()

        return pos, prob, rstate, data

    def _sample(self, p0, prob=None, rstate=None, datas=None, burnin=False):
        """
        Run the emcee sampler for the burnin to create walker which are independent form their starting position
        """
        stg = self.storageUtil.burnin_storage if burnin else self.storageUtil.sample_storage
        niter = self.burninIterations if burnin else self.sampleIterations

        _lastprob = prob if prob is None else [0] * len(p0)

        for pos, prob, rstate, datas in self._sampler.sample(
            p0,
            iterations=niter - stg.iteration,
            lnprob0=prob, rstate0=rstate, blobs0=datas
        ):
            if self.isMaster():
                # Need to grow the storage first
                if not stg.iteration:
                    stg.grow(niter - stg.iteration, datas[0])

                # If we are continuing sampling, we need to grow it more.
                if stg.size < niter:
                    stg.grow(niter - stg.size, datas[0])

                self.storageUtil.persistValues(pos, prob, datas, accepted= prob != _lastprob, random_state=rstate,
                                               burnin=burnin)
                if stg.iteration % 10 == 0:
                    self.log("Iteration finished:" + str(stg.iteration))

                _lastprob = 1*prob

                if self.stopCriteriaStrategy.hasFinished():
                    break

        return pos, prob, rstate, datas

    def sampleBurnin(self, p0, prob=None, rstate=None, datas=None):
        return self._sample(p0, prob, rstate, datas, burnin=True)

    def sample(self, burninPos, burninProb=None, burninRstate=None, datas=None):
        return self._sample(burninPos, burninProb, burninRstate, datas)