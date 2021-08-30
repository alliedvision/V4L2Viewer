pipeline {
	agent { label 'OSN-TF1-BUILD1' }

	triggers {
    pollSCM('H/15 * * * *')
  }

	stages {
		stage('Build') {
			steps {
        dir('build') {
          sh "sudo -E chroot /opt/jetpack-root bash -c 'cd ${env.WORKSPACE} && ./build-and-package.sh'"
          sh "sudo chown -R jenkins:jenkins ."
        }
			}
		}
    stage('Archive') {
      steps {
        dir('build') {
          sh "/opt/bin/archive-artifact V4L2Viewer*.tar.bz2"
        }
      }
    }
	}
}
